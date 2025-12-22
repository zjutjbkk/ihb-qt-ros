#ifndef QTNODESETTINGS_H
#define QTNODESETTINGS_H


#include <QTimer>
#include <QWidget>
#include <QLineEdit>
#include <QImage>
#include <QThread>
#include <QList>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/image.hpp"
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include "ihb_msgs/msg/ihb_status.hpp"
#include "ihb_msgs/msg/muscle_joint_status.hpp"
#include "ihb_msgs/msg/servo_motor_joint_status.hpp"
#include "ihb_msgs/msg/stepper_motor_joint_status.hpp"
#include "ihb_msgs/msg/force_sensor_status.hpp"
#include "ihb_msgs/msg/muscle_joint_target.hpp"
#include "ihb_msgs/msg/servo_motor_joint_target.hpp"
#include "ihb_msgs/msg/stepper_motor_joint_target.hpp"
#include "ihb_msgs/msg/arm_target.hpp"

#include "ihb_msgs/msg/ihb_status.hpp"

class RosThread : public QThread
{
public:
  RosThread() : node_(std::make_shared<rclcpp::Node>("qt_node")) {}

  // add pub
  

  void run() override
  {
    rclcpp::spin(node_);
  }

  rclcpp::Node::SharedPtr get_node()
  {
    return node_;
  }

private:
  rclcpp::Node::SharedPtr node_;
};

namespace Ui {
class QtNodeSettings;
}

class QtNodeSettings : public QWidget {
  Q_OBJECT

public:
  explicit QtNodeSettings(QWidget *parent = nullptr);
  ~QtNodeSettings();

private slots:
  // state callback
  void node_state_update(void);  // 更新状态

  // topic callback
  void topic_state_update(void);  // 更新topic状态

  void on_ButtonIhbStatus_clicked();

  void detection_image_callback(const sensor_msgs::msg::Image::SharedPtr);
  void raw_image_callback(const sensor_msgs::msg::Image::SharedPtr);

  // 从LiveShow接收的订阅控制槽函数
  void handleSubscribeRawImage(QString topic_name);
  void handleUnsubscribeRawImage();
  void handleSubscribeDetectionResult(QString topic_name);
  void handleUnsubscribeDetectionResult();

  // 批量消息处理槽函数 (在 ROS 线程中处理,带延时,不阻塞 GUI)
  void handleServoMotorBatch(QList<ihb_msgs::msg::ServoMotorJointTarget> msgs);
  void handleMuscleJointBatch(QList<ihb_msgs::msg::MuscleJointTarget> msgs);

  // 机械臂目标消息处理槽函数 (新架构)
  void handleLeftArmTarget(ihb_msgs::msg::ArmTarget msg);
  void handleRightArmTarget(ihb_msgs::msg::ArmTarget msg);

private:
  
  Ui::QtNodeSettings *ui;


  // 程序测试时留下的，作为程序状态
  QTimer *timer_;
  //cout
  // std::int
  size_t count_ = 0;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr state_publisher_;

  // node
  // QtNode *qt_node_;
  RosThread *ros_thread_;
  rclcpp::Node::SharedPtr node_;
  cv::Mat  image_;


  // 所有topic line edit 的 指针数组
  QList<QLineEdit *> topic_line_edit_list;

  // 订阅者
  rclcpp::Subscription<ihb_msgs::msg::IhbStatus>::SharedPtr ihb_state_subscriber_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr detection_result_subscriber_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr raw_image_subscriber_;

  // motor target pub
  rclcpp::Publisher<ihb_msgs::msg::ServoMotorJointTarget>::SharedPtr ihb_servo_motor_joint_target_publisher_;
  // motor target pub
  rclcpp::Publisher<ihb_msgs::msg::StepperMotorJointTarget>::SharedPtr ihb_stepper_motor_joint_target_publisher_;
  // muscle target pub
  rclcpp::Publisher<ihb_msgs::msg::MuscleJointTarget>::SharedPtr ihb_muscle_joint_target_publisher_;

  // 机械臂目标发布者 (新架构: 通过话题命名空间区分左右臂)
  rclcpp::Publisher<ihb_msgs::msg::ArmTarget>::SharedPtr left_arm_target_publisher_;
  rclcpp::Publisher<ihb_msgs::msg::ArmTarget>::SharedPtr right_arm_target_publisher_;

};


#endif // QTNODESETTINGS_H
