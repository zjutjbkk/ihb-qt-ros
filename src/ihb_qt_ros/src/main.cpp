
#include "ihb.h"

#include <QApplication>

#include <QLocale>
#include <QTranslator>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "ihb_msgs/msg/ihb_status.hpp"
#include "ihb_msgs/msg/muscle_joint_status.hpp"
#include "ihb_msgs/msg/servo_motor_joint_status.hpp"
#include "ihb_msgs/msg/stepper_motor_joint_status.hpp"
#include "ihb_msgs/msg/force_sensor_status.hpp"
#include "ihb_msgs/msg/muscle_joint_target.hpp"
#include "ihb_msgs/msg/servo_motor_joint_target.hpp"
#include "ihb_msgs/msg/stepper_motor_joint_target.hpp"
#include "ihb_msgs/msg/arm_target.hpp"

#include <QMetaType>
#include <QDebug>

int main(int argc, char *argv[])
{
    // 注册自定义消息类型 后面才能使用connect连接 
    qRegisterMetaType<ihb_msgs::msg::IhbStatus>("ihb_msgs::msg::IhbStatus");
    qRegisterMetaType<ihb_msgs::msg::MuscleJointStatus>("ihb_msgs::msg::MuscleJointStatus");
    qRegisterMetaType<ihb_msgs::msg::ServoMotorJointStatus>("ihb_msgs::msg::ServoMotorJointStatus");
    qRegisterMetaType<ihb_msgs::msg::StepperMotorJointStatus>("ihb_msgs::msg::StepperMotorJointStatus");
    qRegisterMetaType<ihb_msgs::msg::ForceSensorStatus>("ihb_msgs::msg::ForceSensorStatus");
    qRegisterMetaType<ihb_msgs::msg::MuscleJointTarget>("ihb_msgs::msg::MuscleJointTarget");
    qRegisterMetaType<ihb_msgs::msg::ServoMotorJointTarget>("ihb_msgs::msg::ServoMotorJointTarget");
    qRegisterMetaType<ihb_msgs::msg::StepperMotorJointTarget>("ihb_msgs::msg::StepperMotorJointTarget");
    qRegisterMetaType<ihb_msgs::msg::ArmTarget>("ihb_msgs::msg::ArmTarget");

    // 设置ROS_DOMAIN_ID
    putenv((char*)"ROS_DOMAIN_ID=77");

    // 初始化ROS节点
    rclcpp::init(argc, argv);
    qDebug() << "ROS_DOMAIN_ID" << getenv("ROS_DOMAIN_ID");

    QApplication a(argc, argv);
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "IHB_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    IHB w;
    w.show();
    return a.exec();
}
