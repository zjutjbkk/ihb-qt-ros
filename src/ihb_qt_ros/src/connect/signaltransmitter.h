#ifndef SIGNALTRANSMITTER_H
#define SIGNALTRANSMITTER_H

#include <QObject>
#include <QImage>
#include <QList>
#include "ihb_msgs/msg/ihb_status.hpp"
#include "ihb_msgs/msg/muscle_joint_status.hpp"
#include "ihb_msgs/msg/servo_motor_joint_status.hpp"
#include "ihb_msgs/msg/stepper_motor_joint_status.hpp"
#include "ihb_msgs/msg/force_sensor_status.hpp"
#include "ihb_msgs/msg/muscle_joint_target.hpp"
#include "ihb_msgs/msg/servo_motor_joint_target.hpp"
#include "ihb_msgs/msg/stepper_motor_joint_target.hpp"
#include "ihb_msgs/msg/arm_target.hpp"

// #include "ihb_qt_ros_autogen/"
//如果你想在多层对象中将参数直接从子对象发送到第一层对象，而不需要逐层转发信号，可以使用全局信号与槽连接来实现。以下是一个示例：
//全局信号传递
class SignalTransmitter : public QObject
{
    Q_OBJECT
public:
    static SignalTransmitter* instance()
    {
        static SignalTransmitter signalTransmitter;
        return &signalTransmitter;
    }
    bool serialState = false;

private:
    SignalTransmitter() {}

signals:
    void SerialRxDataChangedSignal(QString data);
    void SerialTxDataChangedSignal(QString data);
    void TcpRxDataChangedSignal(QString data);
    void TcpTxDataChangedSignal(QString data);
    void ImageChangedSignal(QImage image);
    void IhbStatusChangedSignal(ihb_msgs::msg::IhbStatus state);

    // 相机图像信号
    void RawImageChangedSignal(QImage image);  // 原始图像信号
    void DetectionResultImageChangedSignal(QImage image);  // 检测结果图像信号

    // 话题订阅控制信号（从LiveShow发送到QtNodeSettings）
    void SubscribeRawImageSignal(QString topic_name);  // 订阅原始图像话题
    void UnsubscribeRawImageSignal();  // 取消订阅原始图像话题
    void SubscribeDetectionResultSignal(QString topic_name);  // 订阅检测结果话题
    void UnsubscribeDetectionResultSignal();  // 取消订阅检测结果话题

    // 命名规则：ihb + 类型 + TargetChangedSignal
    // 单条消息信号 (保留兼容性)
    void ihbServoMotorTargetChangedSignal(ihb_msgs::msg::ServoMotorJointTarget msg);
    void ihbMuscleTargetChangedSignal(ihb_msgs::msg::MuscleJointTarget msg);
    void ihbStepperTargetChangedSignal(ihb_msgs::msg::StepperMotorJointTarget msg);

    // 批量消息信号 (新增,用于避免 GUI 线程阻塞)
    void ihbServoMotorTargetBatchSignal(QList<ihb_msgs::msg::ServoMotorJointTarget> msgs);
    void ihbMuscleTargetBatchSignal(QList<ihb_msgs::msg::MuscleJointTarget> msgs);

    // 机械臂目标消息信号 (新架构: 分层组合设计)
    void ihbLeftArmTargetSignal(ihb_msgs::msg::ArmTarget msg);
    void ihbRightArmTargetSignal(ihb_msgs::msg::ArmTarget msg);
};


#endif // SIGNALTRANSMITTER_H
