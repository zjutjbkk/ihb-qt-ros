#include "qtnodesettings.h"
#include "ui_qtnodesettings.h"
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/image.hpp"
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.hpp>
// thread
#include "ihb_msgs/msg/ihb_status.hpp"
#include "ihb_msgs/msg/muscle_joint_status.hpp"
#include "ihb_msgs/msg/servo_motor_joint_status.hpp"
#include "ihb_msgs/msg/stepper_motor_joint_status.hpp"
#include "ihb_msgs/msg/force_sensor_status.hpp"
#include "ihb_msgs/msg/muscle_joint_target.hpp"
#include "ihb_msgs/msg/servo_motor_joint_target.hpp"
#include "ihb_msgs/msg/stepper_motor_joint_target.hpp"
#include "ihb_msgs/msg/arm_target.hpp"

// #include "qtnode.h"
// state LED
#include "signaltransmitter.h"

void setLED(QLabel *label, int color, int size = 10)
{
  //    label->setText("");
  QString min_width = QString("min-width: %1px;").arg(size);
  QString min_height = QString("min-height: %1px;").arg(size);
  QString max_width = QString("max-width: %1px;").arg(size);
  QString max_height = QString("max-height: %1px;").arg(size);

  QString border_radius = QString("border-radius: %1px;").arg(size / 2);
  //    QString border = QString("border:1px solid black;");

  QString background = "background-color:";
  switch (color)
  {

  case 0:
    // 红色
    background += "rgb(255, 0, 0)";
    break;
  case 1:
    // 绿色
    background += "rgb(20, 185, 20)";
    break;
  case 2:
    // 灰色
    background += "rgb(190, 190, 190)";
    break;
  default:
    break;
  }
  const QString Sheetstyle1 = min_width + min_height + max_width + max_height +
                              border_radius + background;
  label->setStyleSheet(Sheetstyle1);
}

QtNodeSettings::QtNodeSettings(QWidget *parent)
    : QWidget(parent), ui(new Ui::QtNodeSettings)
{
  ui->setupUi(this);

  ros_thread_ = new RosThread();
  ros_thread_->start();
  node_ = ros_thread_->get_node();

  //  setLED(ui->label_5, 2);
  // 获取系统ROS_DOMAIN_ID
  char *ros_domain_id = getenv("ROS_DOMAIN_ID");
  if (ros_domain_id == NULL)
  {
    RCLCPP_INFO(node_->get_logger(), "ROS_DOMAIN_ID is NULL");
    // 设置默认值为14
    ui->lineEditDomainID->setText("14");
    setLED(ui->LEDDomainID, 0);  // 设置LED为红色，表示使用了默认值
  }
  else
  {
    RCLCPP_INFO(node_->get_logger(), "ROS_DOMAIN_ID is %s", ros_domain_id);
    ui->lineEditDomainID->setText(QString(ros_domain_id));
    setLED(ui->LEDDomainID, 1);  // 设置LED为绿色
  }
  // 创建后台线程并运行ros rcl spin

  state_publisher_ = node_->create_publisher<std_msgs::msg::String>("ihb_qt_state_topic", 10); // 这里的10是什么意思？？？ 10 是一个队列的长度，表示最多可以缓存10个消息
  //  timer_ = this->create_wall_timer(std::chrono::milliseconds(500),
  //  std::bind(&QtNodeSettings::timer_callback, this));
  // 使用qt内置的定时器
  timer_ = new QTimer(this);
  connect(timer_, SIGNAL(timeout()), this, SLOT(node_state_update()));
  timer_->start(1000);

  // 遍历所有的line edit 如果是topic的line edit 就加入到list中
  QList<QLineEdit *> all_line_edit = this->findChildren<QLineEdit *>();
  for (int i = 0; i < all_line_edit.length(); i++)
  {
    if (all_line_edit[i]->objectName().contains("Topic"))
    {
      topic_line_edit_list.append(all_line_edit[i]);
    }
  }
  // 定时器 检查topic状态
  QTimer *topic_state_timer = new QTimer(this);
  connect(topic_state_timer, SIGNAL(timeout()), this, SLOT(topic_state_update()));
  topic_state_timer->start(1000);
  // 设置发布者
  ihb_servo_motor_joint_target_publisher_ = node_->create_publisher<ihb_msgs::msg::ServoMotorJointTarget>("ihb_servo_motor_joint_target", 10);
  // 设置发布者
  ihb_stepper_motor_joint_target_publisher_ = node_->create_publisher<ihb_msgs::msg::StepperMotorJointTarget>("ihb_stepper_motor_joint_target", 10);
  // 设置发布者
  ihb_muscle_joint_target_publisher_ = node_->create_publisher<ihb_msgs::msg::MuscleJointTarget>("ihb_muscle_joint_target", 10);

  // 设置机械臂目标发布者 (新架构: 通过话题命名空间区分左右臂)
  left_arm_target_publisher_ = node_->create_publisher<ihb_msgs::msg::ArmTarget>("/left_arm/target", 10);
  right_arm_target_publisher_ = node_->create_publisher<ihb_msgs::msg::ArmTarget>("/right_arm/target", 10);

  // 获取信号传递类
  SignalTransmitter *signal_transmitter = SignalTransmitter::instance();

  // 连接来自LiveShow的订阅控制信号
  connect(signal_transmitter, &SignalTransmitter::SubscribeRawImageSignal,
          this, &QtNodeSettings::handleSubscribeRawImage);
  connect(signal_transmitter, &SignalTransmitter::UnsubscribeRawImageSignal,
          this, &QtNodeSettings::handleUnsubscribeRawImage);
  connect(signal_transmitter, &SignalTransmitter::SubscribeDetectionResultSignal,
          this, &QtNodeSettings::handleSubscribeDetectionResult);
  connect(signal_transmitter, &SignalTransmitter::UnsubscribeDetectionResultSignal,
          this, &QtNodeSettings::handleUnsubscribeDetectionResult);

  // 信号传递类的信号
  // motor joint target 匿名函数
  connect(signal_transmitter, &SignalTransmitter::ihbServoMotorTargetChangedSignal, this, [=](ihb_msgs::msg::ServoMotorJointTarget motor_joint_target)
          {
            //
            // log
            RCLCPP_INFO(node_->get_logger(), "Motor joint target changed");
            // 发布消息
            // 创建消息
            ihb_msgs::msg::ServoMotorJointTarget message = ihb_msgs::msg::ServoMotorJointTarget();
            // 设置消息
            message.id = motor_joint_target.id;
            message.angle = motor_joint_target.angle;
            message.speed_limit = motor_joint_target.speed_limit;
            // 设置云深处电机参数
            message.cmd =             motor_joint_target.cmd;
            message.target_angle =    motor_joint_target.target_angle;
            message.target_velocity = motor_joint_target.target_velocity;
            message.kp =              motor_joint_target.kp;
            message.kd =              motor_joint_target.kd;
            message.target_torque =   motor_joint_target.target_torque;
            // 发布消息
            ihb_servo_motor_joint_target_publisher_->publish(message); });
  // stepper motor joint target 匿名函数
  connect(signal_transmitter, &SignalTransmitter::ihbStepperTargetChangedSignal, this, [=](ihb_msgs::msg::StepperMotorJointTarget stepper_motor_joint_target)
          {
            //
            // log
            RCLCPP_INFO(node_->get_logger(), "Stepper motor joint target changed");
            // 发布消息
            // 创建消息
            ihb_msgs::msg::StepperMotorJointTarget message = ihb_msgs::msg::StepperMotorJointTarget();
            // 设置消息
            message.id = stepper_motor_joint_target.id;
            message.height = stepper_motor_joint_target.height;
            // 发布消息
            ihb_stepper_motor_joint_target_publisher_->publish(message); });
  // muscle joint target 匿名函数
  connect(signal_transmitter, &SignalTransmitter::ihbMuscleTargetChangedSignal, this, [=](ihb_msgs::msg::MuscleJointTarget muscle_joint_target)
          {
            //
            // log
            RCLCPP_INFO(node_->get_logger(), "Muscle joint target changed");
                //  RCLCPP_INFO(node_->get_logger(), "wxxxx");
                RCLCPP_INFO(node_->get_logger(), "id: %d", muscle_joint_target.id);
            // 发布消息
            // 创建消息
            ihb_msgs::msg::MuscleJointTarget message = ihb_msgs::msg::MuscleJointTarget();
            // 设置消息
            message.id = muscle_joint_target.id;
            message.cmd = muscle_joint_target.cmd;
            message.angle[0] = muscle_joint_target.angle[0];
            message.angle[1] = muscle_joint_target.angle[1];
            message.pressure[0] = muscle_joint_target.pressure[0];
            message.pressure[1] = muscle_joint_target.pressure[1];
            message.pressure[2] = muscle_joint_target.pressure[2];
            message.pressure[3] = muscle_joint_target.pressure[3];
            message.stiffness[0] = muscle_joint_target.stiffness[0];
            message.stiffness[1] = muscle_joint_target.stiffness[1];
            // ✅ 修复：添加 PID 参数字段
            message.kp = muscle_joint_target.kp;
            message.ki = muscle_joint_target.ki;
            message.kd = muscle_joint_target.kd;

            // 调试日志：打印 PID 参数
            RCLCPP_INFO(node_->get_logger(), "PID params - Kp: %.2f, Ki: %.2f, Kd: %.2f",
                       message.kp, message.ki, message.kd);

            // 发布消息
            ihb_muscle_joint_target_publisher_->publish(message); });

  // 批量消息处理 - 连接信号到槽函数
  connect(signal_transmitter, &SignalTransmitter::ihbServoMotorTargetBatchSignal,
          this, &QtNodeSettings::handleServoMotorBatch);
  connect(signal_transmitter, &SignalTransmitter::ihbMuscleTargetBatchSignal,
          this, &QtNodeSettings::handleMuscleJointBatch);

  // 机械臂目标消息处理 - 连接信号到槽函数 (新架构)
  connect(signal_transmitter, &SignalTransmitter::ihbLeftArmTargetSignal,
          this, &QtNodeSettings::handleLeftArmTarget);
  connect(signal_transmitter, &SignalTransmitter::ihbRightArmTargetSignal,
          this, &QtNodeSettings::handleRightArmTarget);
}

QtNodeSettings::~QtNodeSettings()
{
  // delete qt_node_thread

  delete ui;
}

void QtNodeSettings::node_state_update()
{
  //别发布了 看着烦
  // auto message = std_msgs::msg::String();
  // message.data = "Hello, world! " + std::to_string(count_++);
  // RCLCPP_INFO(node_->get_logger(), "Publishing: '%s'", message.data.c_str());
  // state_publisher_->publish(message);
}

void QtNodeSettings::topic_state_update(void)
{

  // DOMAIN ID
  // 获取系统ROS_DOMAIN_ID
  char *ros_domain_id = getenv("ROS_DOMAIN_ID");
  if (ros_domain_id == NULL)
  {
    // RCLCPP_INFO(node_->get_logger(), "ROS_DOMAIN_ID is NULL");
    // 设置默认值为14
    ui->lineEditDomainID->setText("14");
    setLED(ui->LEDDomainID, 0);  // 设置LED为红色，表示使用了默认值
  }
  else
  {
    // RCLCPP_INFO(node_->get_logger(), "ROS_DOMAIN_ID is %s", ros_domain_id);
    ui->lineEditDomainID->setText(QString(ros_domain_id));
    setLED(ui->LEDDomainID, 1);  // 设置LED为绿色
  }

  // 遍历 topic_line_edit_list 并检查topic状态
  for (int i = 0; i < topic_line_edit_list.length(); i++)
  {
    // 获取topic名字
    QString topic_name = topic_line_edit_list[i]->text();
    // log

    // 获取系统中所有的话题
    auto topics = node_->get_topic_names_and_types();

    // 检查话题是否存在
    bool topic_exists = std::any_of(topics.begin(), topics.end(),
                                    [&topic_name](const auto &topic)
                                    {
                                      // LOG
                                      // RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "topic name: %s", topic.first.c_str());
                                      return topic.first == topic_name.toStdString();
                                    });
    // 设置topic状态
    if (topic_exists)
    {
      // 话题存在
      // 检查发布者信息
      auto publishers_info = node_->get_publishers_info_by_topic(topic_name.toStdString());
      bool is_publisher = !publishers_info.empty();
      if (is_publisher)
      {
        QString LED_object_name = "LED" + topic_line_edit_list[i]->objectName().remove("lineEdit");
        QLabel *LED = this->findChild<QLabel *>(LED_object_name);
        // 判断LED对象是否存在
        if (LED != NULL)
        {
          // LED对象存在
          // 设置LED状态
          setLED(LED, 1);
        }
      }
      else
      {
        // 话题存在，但是没有发布者
        QString LED_object_name = "LED" + topic_line_edit_list[i]->objectName().remove("lineEdit");
        QLabel *LED = this->findChild<QLabel *>(LED_object_name);
        // 判断LED对象是否存在
        if (LED != NULL)
        {
          // LED对象存在
          // 设置LED状态
          setLED(LED, 0);
        }
      }
    }
    else
    {
      // 话题不存在
      // LOG
      // RCLCPP_INFO(node_->get_logger(), "Topic %s does not exist", topic_name.toStdString().c_str());
      // same as above
      QString LED_object_name = "LED" + topic_line_edit_list[i]->objectName().remove("lineEdit");
      QLabel *LED = this->findChild<QLabel *>(LED_object_name);
      if (LED != NULL)
      {
        setLED(LED, 0);
      }
    }
  }
}

// void QtNodeSettings::on_pushButton_clicked() { timer_callback(); }

void QtNodeSettings::on_ButtonIhbStatus_clicked()
{
  // 切换订阅状态
  if (ihb_state_subscriber_ == nullptr)
  {
    // 创建订阅者
    rclcpp::QoS qos(rclcpp::KeepLast(10));

    ihb_state_subscriber_ = node_->create_subscription<ihb_msgs::msg::IhbStatus>(
        ui->lineEditStatePubTopic->text().toStdString().c_str(), qos,
        [this](const ihb_msgs::msg::IhbStatus::SharedPtr msg) -> void
        {
          ihb_msgs::msg::IhbStatus ihb_status = *msg;
          // 获取信号传递类并触发信号
          SignalTransmitter *signal_transmitter = SignalTransmitter::instance();
          emit signal_transmitter->IhbStatusChangedSignal(ihb_status);
        });

    ui->ButtonIhbStatus->setText("停止订阅");
    RCLCPP_INFO(node_->get_logger(), "开始订阅 IHB Status 话题: %s",
                ui->lineEditStatePubTopic->text().toStdString().c_str());
  }
  else
  {
    // 销毁订阅者
    ihb_state_subscriber_.reset();
    ui->ButtonIhbStatus->setText("开始订阅");
    RCLCPP_INFO(node_->get_logger(), "停止订阅 IHB Status 话题");
  }
}


void QtNodeSettings::detection_image_callback(const sensor_msgs::msg::Image::SharedPtr msg)
{
  // 将ROS图像消息转换为OpenCV格式
  cv_bridge::CvImagePtr cv_ptr;
  try
  {
    // 检测结果图像应该是BGR8格式
    cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
  }
  catch (cv_bridge::Exception &e)
  {
    RCLCPP_ERROR(node_->get_logger(), "cv_bridge exception: %s", e.what());
    return;
  }

  // 将OpenCV图像转换为QImage
  cv::Mat image = cv_ptr->image;
  QImage qimage((const unsigned char *)(image.data),
                image.cols,
                image.rows,
                image.step,
                QImage::Format_BGR888);

  // 创建QImage的深拷贝，避免数据被释放
  QImage qimage_copy = qimage.copy();

  // 通过SignalTransmitter发送图像信号
  SignalTransmitter *signal_transmitter = SignalTransmitter::instance();
  emit signal_transmitter->DetectionResultImageChangedSignal(qimage_copy);

  // RCLCPP_INFO(node_->get_logger(), "检测结果图像已发送: %d x %d", image.cols, image.rows);
}

void QtNodeSettings::raw_image_callback(const sensor_msgs::msg::Image::SharedPtr msg)
{
  // 添加调试日志（每10帧打印一次）
  static int callback_count = 0;
  if (++callback_count % 1000 == 0) {
    RCLCPP_INFO(node_->get_logger(), "原始图像回调被触发 (第%d次)，编码: %s, 尺寸: %d x %d",
                callback_count, msg->encoding.c_str(), msg->width, msg->height);
  }

  // 将ROS图像消息转换为OpenCV格式
  cv_bridge::CvImagePtr cv_ptr;
  try
  {
    // 原始图像是RGB8格式，直接使用原始编码，然后手动转换为BGR8
    // 这样比让cv_bridge自动转换更可靠
    if (msg->encoding == "rgb8")
    {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::RGB8);
      // 手动转换RGB8到BGR8
      cv::cvtColor(cv_ptr->image, cv_ptr->image, cv::COLOR_RGB2BGR);
    }
    else
    {
      // 其他格式尝试自动转换为BGR8
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
  }
  catch (cv_bridge::Exception &e)
  {
    RCLCPP_ERROR(node_->get_logger(), "cv_bridge转换失败: %s (编码: %s)", e.what(), msg->encoding.c_str());
    return;
  }

  // 将OpenCV图像转换为QImage
  cv::Mat image = cv_ptr->image;
  QImage qimage((const unsigned char *)(image.data),
                image.cols,
                image.rows,
                image.step,
                QImage::Format_BGR888);

  // 创建QImage的深拷贝，避免数据被释放
  QImage qimage_copy = qimage.copy();

  // 通过SignalTransmitter发送图像信号
  SignalTransmitter *signal_transmitter = SignalTransmitter::instance();
  emit signal_transmitter->RawImageChangedSignal(qimage_copy);
}

// 批量处理伺服电机消息 (在 ROS 线程中执行,带延时,不阻塞 GUI)
void QtNodeSettings::handleServoMotorBatch(QList<ihb_msgs::msg::ServoMotorJointTarget> msgs)
{
  RCLCPP_INFO(node_->get_logger(), "Handling servo motor batch: %d messages", msgs.size());

  for (const auto& motor_joint_target : msgs) {
    // 创建消息
    ihb_msgs::msg::ServoMotorJointTarget message = ihb_msgs::msg::ServoMotorJointTarget();

    // 设置消息
    message.id = motor_joint_target.id;
    message.angle = motor_joint_target.angle;
    message.speed_limit = motor_joint_target.speed_limit;
    message.cmd = motor_joint_target.cmd;
    message.target_angle = motor_joint_target.target_angle;
    message.target_velocity = motor_joint_target.target_velocity;
    message.kp = motor_joint_target.kp;
    message.kd = motor_joint_target.kd;
    message.target_torque = motor_joint_target.target_torque;

    // 发布消息
    ihb_servo_motor_joint_target_publisher_->publish(message);
    RCLCPP_INFO(node_->get_logger(), "Published servo motor target: id=%d", message.id);

    // 添加延时,确保消息被正确处理 (在 ROS 线程中,不阻塞 GUI)
    QThread::msleep(1);  // 1毫秒延时
  }

  RCLCPP_INFO(node_->get_logger(), "Servo motor batch completed");
}

// 批量处理肌肉关节消息 (在 ROS 线程中执行,带延时,不阻塞 GUI)
void QtNodeSettings::handleMuscleJointBatch(QList<ihb_msgs::msg::MuscleJointTarget> msgs)
{
  RCLCPP_INFO(node_->get_logger(), "Handling muscle joint batch: %d messages", msgs.size());

  for (const auto& muscle_joint_target : msgs) {
    // 创建消息
    ihb_msgs::msg::MuscleJointTarget message = ihb_msgs::msg::MuscleJointTarget();

    // 设置消息
    message.id = muscle_joint_target.id;
    message.angle[0] = muscle_joint_target.angle[0];
    message.angle[1] = muscle_joint_target.angle[1];
    message.pressure[0] = muscle_joint_target.pressure[0];
    message.pressure[1] = muscle_joint_target.pressure[1];
    message.pressure[2] = muscle_joint_target.pressure[2];
    message.pressure[3] = muscle_joint_target.pressure[3];
    message.stiffness[0] = muscle_joint_target.stiffness[0];
    message.stiffness[1] = muscle_joint_target.stiffness[1];
    message.cmd = muscle_joint_target.cmd;

    // 发布消息
    ihb_muscle_joint_target_publisher_->publish(message);
    RCLCPP_INFO(node_->get_logger(), "Published muscle joint target: id=%d", message.id);

    // 添加延时,确保消息被正确处理 (在 ROS 线程中,不阻塞 GUI)
    QThread::msleep(1);  // 1毫秒延时
  }

  RCLCPP_INFO(node_->get_logger(), "Muscle joint batch completed");
}

// 处理左臂目标消息 (新架构)
void QtNodeSettings::handleLeftArmTarget(ihb_msgs::msg::ArmTarget msg)
{
  RCLCPP_INFO(node_->get_logger(), "Handling left arm target: cmd=%d", msg.cmd);

  // 发布到 /left_arm/target 话题
  left_arm_target_publisher_->publish(msg);

  RCLCPP_INFO(node_->get_logger(), "Published left arm target: cmd=%d, servo_joints=%zu, muscle_joints=%zu",
              msg.cmd, msg.servo_joints.size(), msg.muscle_joints.size());
}

// 处理右臂目标消息 (新架构)
void QtNodeSettings::handleRightArmTarget(ihb_msgs::msg::ArmTarget msg)
{
  RCLCPP_INFO(node_->get_logger(), "Handling right arm target: cmd=%d", msg.cmd);

  // 发布到 /right_arm/target 话题
  right_arm_target_publisher_->publish(msg);

  RCLCPP_INFO(node_->get_logger(), "Published right arm target: cmd=%d, servo_joints=%zu, muscle_joints=%zu",
              msg.cmd, msg.servo_joints.size(), msg.muscle_joints.size());
}

// 处理来自LiveShow的订阅原始图像请求
void QtNodeSettings::handleSubscribeRawImage(QString topic_name)
{
  if (raw_image_subscriber_ == nullptr)
  {
    // 创建订阅者，使用RELIABLE QoS以匹配相机发布者
    // 相机发布者使用RELIABLE，订阅者必须使用RELIABLE或更高级别才能接收数据
    rclcpp::QoS qos(rclcpp::KeepLast(1));
    qos.reliability(rclcpp::ReliabilityPolicy::Reliable);  // 修改为Reliable
    qos.durability(rclcpp::DurabilityPolicy::Volatile);

    raw_image_subscriber_ = node_->create_subscription<sensor_msgs::msg::Image>(
        topic_name.toStdString().c_str(), qos,
        [this](const sensor_msgs::msg::Image::SharedPtr msg) -> void
        {
          raw_image_callback(msg);
        });

    RCLCPP_INFO(node_->get_logger(), "开始订阅原始图像话题: %s (QoS: RELIABLE)", topic_name.toStdString().c_str());
  }
}

// 处理来自LiveShow的取消订阅原始图像请求
void QtNodeSettings::handleUnsubscribeRawImage()
{
  if (raw_image_subscriber_ != nullptr)
  {
    raw_image_subscriber_.reset();
    RCLCPP_INFO(node_->get_logger(), "停止订阅原始图像话题");
  }
}

// 处理来自LiveShow的订阅检测结果请求
void QtNodeSettings::handleSubscribeDetectionResult(QString topic_name)
{
  if (detection_result_subscriber_ == nullptr)
  {
    // 创建订阅者，使用BEST_EFFORT QoS以匹配相机发布者
    rclcpp::QoS qos(rclcpp::KeepLast(1));
    qos.reliability(rclcpp::ReliabilityPolicy::BestEffort);

    detection_result_subscriber_ = node_->create_subscription<sensor_msgs::msg::Image>(
        topic_name.toStdString().c_str(), qos,
        [this](const sensor_msgs::msg::Image::SharedPtr msg) -> void
        {
          detection_image_callback(msg);
        });

    RCLCPP_INFO(node_->get_logger(), "开始订阅检测结果话题: %s", topic_name.toStdString().c_str());
  }
}

// 处理来自LiveShow的取消订阅检测结果请求
void QtNodeSettings::handleUnsubscribeDetectionResult()
{
  if (detection_result_subscriber_ != nullptr)
  {
    detection_result_subscriber_.reset();
    RCLCPP_INFO(node_->get_logger(), "停止订阅检测结果话题");
  }
}
