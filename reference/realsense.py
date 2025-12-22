import cv2
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image, CameraInfo
from cv_bridge import CvBridge
from rcl_interfaces.msg import ParameterDescriptor
from vision_msgs.msg import Detection3DArray, Detection3D, ObjectHypothesisWithPose
from geometry_msgs.msg import Point, Pose
import numpy as np
import time
import os
from .utils import Colors, yaml_load, preprocessing_img, do_NMS, scale_boxes, box_label
from message_filters import ApproximateTimeSynchronizer, Subscriber
import torch
import warnings
from ultralytics import YOLO # 从 ultralytics 导入 YOLO
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSHistoryPolicy
from rclpy.duration import Duration



class YOLOv5Node(Node):
    def __init__(self):
        super().__init__('realense_node')
        qos_profile = QoSProfile(
            reliability=QoSReliabilityPolicy.BEST_EFFORT,
            history=QoSHistoryPolicy.KEEP_LAST,
            depth=1
        )
        # Declare parameters
        self.declare_parameter("model_path", "/home/wx/servicerobot/vision/src/ros2_yolov5/ros2_yolov5/yolo11m.pt", 
            ParameterDescriptor(name="model_path", description="Path to YOLO model file")) # 修改描述
        
        self.declare_parameter("conf_threshold", 0.45, 
            ParameterDescriptor(name="conf_threshold", description="Confidence threshold"))
        
        self.declare_parameter("iou_threshold", 0.25, 
            ParameterDescriptor(name="iou_threshold", description="IOU threshold for NMS"))
        
        self.declare_parameter("input_topic", "/camera/camera/color/image_raw", 
            ParameterDescriptor(name="input_topic", description="Input image topic"))
        
        self.declare_parameter("show_result", False, 
            ParameterDescriptor(name="show_result", description="Whether to show detection results"))
        self.declare_parameter("depth_topic", "/camera/camera/depth/image_rect_raw", 
            ParameterDescriptor(name="depth_topic", description="Input depth image topic"))
        # 添加深度处理相关的参数
        self.declare_parameter("depth_scale", 0.001,  # 对于RealSense相机，深度单位通常是毫米
            ParameterDescriptor(name="depth_scale", description="Depth scale factor to convert to meters"))
        
        self.declare_parameter("depth_trunc", 2,  # 最大深度阈值（米）
            ParameterDescriptor(name="depth_trunc", description="Maximum depth in meters"))

        # Load parameters
        model_path = self.get_parameter('model_path').value
        self.conf_thres = self.get_parameter('conf_threshold').value
        self.iou_thres = self.get_parameter('iou_threshold').value
        input_topic = self.get_parameter('input_topic').value
        self.show_result = self.get_parameter('show_result').value
                # 加载新参数
        self.depth_scale = self.get_parameter('depth_scale').value
        self.depth_trunc = self.get_parameter('depth_trunc').value

        # Initialize model
        if not model_path:
            self.get_logger().error("Model path not specified!")
            raise ValueError("Model path parameter is required")
        
        try:
            # Select device
            self.device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
            
            # Load model using YOLO from ultralytics
            self.model = YOLO(model_path) # 修改模型加载方式
            self.model.to(self.device)
            # self.model.eval() # YOLO 对象不需要显式调用 eval()
            
            self.get_logger().info(f"Model loaded successfully from {model_path}")
            
        except Exception as e:
            self.get_logger().error(f"Failed to load model: {str(e)}")
            raise

        # Initialize other components
        self.cv_bridge = CvBridge()
        # self.colors = Colors() # 移除不再需要的 Colors
        self.new_shape = (640, 640)  # YOLO input shape

                
        
        # 设置时间同步阈值（秒）
        self.sync_threshold = 0.1
        
        # Load class names from model
        self.names = self.model.names

        # Camera intrinsics (will be updated when receiving camera info)
        self.camera_matrix = None
        self.dist_coeffs = None
        self.camera_info_received = False

        # Create publishers
        self.result_img_pub = self.create_publisher(Image, 'detection_result', qos_profile=qos_profile)

        self.detection3d_pub = self.create_publisher(Detection3DArray, 'detections_3d', 10)

        # 修改订阅者实现，使用同步订阅
        self.image_sub = Subscriber(self, Image, input_topic)
        self.depth_sub = Subscriber(self, Image, self.get_parameter('depth_topic').value)
        
        # 使用近似时间同步器
        self.ts = ApproximateTimeSynchronizer(
            [self.image_sub, self.depth_sub],
            queue_size=10,
            slop=0.1  # 允许的时间差异(秒)
        )
        self.ts.registerCallback(self.synchronized_callback)
        
        # Subscribe to camera info topic
        self.camera_info_sub = self.create_subscription(
            CameraInfo,
            '/camera/camera/color/camera_info',  # Adjust this topic name as needed
            self.camera_info_callback,
            10
        )
        self.last_publish_time = time.time()
        self.publish_interval = 0.1  # 10 Hz, or 0.2 for 5 Hz
        
        self.get_logger().info("YOLOv5 node initialized successfully")

    def camera_info_callback(self, msg):
        """Handle camera info messages to get intrinsic parameters"""
        if not self.camera_info_received:
            self.camera_matrix = np.array(msg.k).reshape(3, 3)
            self.dist_coeffs = np.array(msg.d)
            self.camera_info_received = True
            self.get_logger().info("Camera intrinsics received")


    
    def get_depth_value(self, x, y, depth_image):
        """获取特定像素位置的深度值，包含基本的噪声处理"""
        if depth_image is None:
            return None
            
        height, width = depth_image.shape
        if not (0 <= x < width and 0 <= y < height):
            return None
            
        # 在目标点周围取一个小窗口进行均值滤波
        window_size = 5
        x_start = max(0, int(x - window_size//2))
        x_end = min(width, int(x + window_size//2))
        y_start = max(0, int(y - window_size//2))
        y_end = min(height, int(y + window_size//2))
        
        depth_window = depth_image[y_start:y_end, x_start:x_end]
        
        # 过滤掉0值和异常值
        valid_depths = depth_window[depth_window > 0]
        valid_depths = valid_depths[valid_depths < self.depth_trunc / self.depth_scale]
        
        if len(valid_depths) == 0:
            return None
            
        # 使用中值滤波获取更稳定的深度值
        depth_mm = np.median(valid_depths)
        depth_m = float(depth_mm) * self.depth_scale
        
        return depth_m

    

    def calculate_3d_point(self, pixel_x, pixel_y, depth):
        """
        Calculate 3D point from pixel coordinates and depth with distortion correction
        
        Args:
            pixel_x (float): x coordinate in pixel space
            pixel_y (float): y coordinate in pixel space
            depth (float): depth value in meters
        
        Returns:
            list: [x, y, z] coordinates in camera frame
        """
        if not self.camera_info_received:
            return None

                # 1. 将像素坐标转换为归一化平面坐标
        pixel_point = np.array([[pixel_x, pixel_y]], dtype=np.float32)
        
        # 2. 畸变校正
        undistorted_point = cv2.undistortPoints(
            pixel_point, 
            self.camera_matrix, 
            self.dist_coeffs, 
            None,
            self.camera_matrix
        )
                # 3. 获取校正后的像素坐标
        corrected_x = undistorted_point[0][0][0]
        corrected_y = undistorted_point[0][0][1]
        

        # Convert pixel coordinates to normalized coordinates
        fx = self.camera_matrix[0, 0]
        fy = self.camera_matrix[1, 1]
        cx = self.camera_matrix[0, 2]
        cy = self.camera_matrix[1, 2]

        # 使用校正后的坐标计算3D点
        x = (corrected_x - cx) * depth / fx
        y = (corrected_y - cy) * depth / fy
        z = depth

        return [x, y, z]

    def synchronized_callback(self, img_msg, depth_msg):
        """处理同步的RGB和深度图像"""
        if not self.camera_info_received:
            self.get_logger().warn_throttle("Waiting for camera info...")
            return

        try:
            # Convert images
            cv_img = self.cv_bridge.imgmsg_to_cv2(img_msg, 'bgr8')
            depth_img = self.cv_bridge.imgmsg_to_cv2(depth_msg, '16UC1')

            # Perform inference using YOLO (与 orbbec.py 一致)
            results = self.model(cv_img, conf=self.conf_thres)[0]
            
            # Prepare 3D detection message
            detection3d_msg = Detection3DArray()
            detection3d_msg.header = img_msg.header
            
            # Create copy of result image
            result_img = cv_img.copy()
            
            # Process detections (与 orbbec.py 一致)
            if len(results.boxes) > 0:
                for box in results.boxes:
                    # Get box coordinates
                    xyxy = box.xyxy[0].cpu().numpy()
                    conf = float(box.conf)
                    cls = int(box.cls)
                    
                    # Ensure coordinates are integers
                    xyxy = [int(x) for x in xyxy]
                    
                    # Calculate box center
                    center_x = int((xyxy[0] + xyxy[2]) / 2)
                    center_y = int((xyxy[1] + xyxy[3]) / 2)
                    
                    
                    # 获取深度值
                    depth = self.get_depth_value(center_x, center_y, depth_img)
                    #打印深度值
                    self.get_logger().info(f"Depth: {depth}")
                    # depth=2
                    
                    if depth is not None and depth > 0:
                        # 计算3D坐标

                        point_3d = self.calculate_3d_point(center_x, center_y, depth)
                        
                        if point_3d is not None:
                            # 绘制边界框和标签
                            # c = int(cls) # cls 已经是 int
                            label = f"{self.names[cls]} {conf:.2f} D:{depth:.2f}m" # 使用 cls
                            # color = self.colors(c, True) # 移除旧的颜色获取方式
                                  # 使用固定颜色进行测试
                            color = (0, 255, 0)  # 绿色
                            
                            # 绘制边界框
                            cv2.rectangle(result_img, (xyxy[0], xyxy[1]), (xyxy[2], xyxy[3]), color, 2)
                            
                            # 绘制标签背景
                            t_size = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.6, 1)[0]
                            cv2.rectangle(result_img, 
                                        (xyxy[0], xyxy[1] - t_size[1] - 3),
                                        (xyxy[0] + t_size[0], xyxy[1]), 
                                        color, 
                                        -1)
                            
                            # 绘制标签文字
                            cv2.putText(result_img, 
                                      label, 
                                      (xyxy[0], xyxy[1] - 2),
                                      cv2.FONT_HERSHEY_SIMPLEX,
                                      0.6,
                                      (255, 255, 255),
                                      1)
                            
                            # 绘制中心点
                            cv2.circle(result_img, (center_x, center_y), 4, (0, 255, 0), -1)
                            
                            # 添加3D检测结果
                            detection3d = Detection3D()
                            detection3d.header = img_msg.header
                            
                            detection3d.results.append(ObjectHypothesisWithPose())
                            detection3d.results[0].hypothesis.class_id = self.names[cls] # 使用 cls
                            detection3d.results[0].hypothesis.score = float(conf)
                            
                            detection3d.results[0].pose.pose.position = Point(
                                x=float(point_3d[0]),
                                y=float(point_3d[1]),
                                z=float(point_3d[2])
                            )
                             # 绘制三维坐标信息
                            coord_label = f"3D: ({point_3d[0]:.2f}, {point_3d[1]:.2f}, {point_3d[2]:.2f})"
                            cv2.putText(result_img, coord_label, (xyxy[0], xyxy[1] - 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
                            self.get_logger().info(f"3D Point: {point_3d}")
                            
                            detection3d_msg.detections.append(detection3d)
            
            # 按频率发布结果图像
            current_time = time.time()
            if (current_time - self.last_publish_time) > self.publish_interval:
                result_img_msg = self.cv_bridge.cv2_to_imgmsg(result_img, 'bgr8')
                result_img_msg.header = img_msg.header
                self.result_img_pub.publish(result_img_msg)
                self.last_publish_time = current_time

            # 显示结果（如果启用）
            if self.show_result:
                cv2.imshow('YOLOv5 Detection Result', result_img)
                cv2.waitKey(1)
                
        except Exception as e:
            self.get_logger().error(f'Error in synchronized_callback: {str(e)}')

def main(args=None):
    warnings.filterwarnings("ignore", category=FutureWarning)
    rclpy.init(args=args)
    node = YOLOv5Node()
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == '__main__':
    main()