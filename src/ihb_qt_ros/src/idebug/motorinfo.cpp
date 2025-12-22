//motorinfo.cpp
#include "motorinfo.h"
#include "ui_motorinfo.h"
#include "src/connect/signaltransmitter.h"
MotorInfo::MotorInfo(QWidget *parent, int id) : QWidget(parent),
                                                ui(new Ui::MotorInfo)
{
    ui->setupUi(this);
    _id = id;
    ui->idLabel->setText(QString::number(id));
}

MotorInfo::~MotorInfo()
{
    delete ui;
}
// void valueChangedSlot(QObject* senderObject){
//     qDebug() << "test";
////
//}
void MotorInfo::setId(int id)
{
    _id = id;
    ui->idLabel->setText(QString::number(id));
}
void MotorInfo::update(ihb_msgs::msg::ServoMotorJointStatus msg)
{
    _angleValue = msg.angle;
    _speedValue = msg.speed;
    _tempValue = msg.temp;
    _currentValue = msg.current;
    _id = msg.id;
    if (msg.is_online == true){_isEnable = true;}
        else{_isEnable = false;};
    ui->idLabel->setText(QString::number(_id));
    ui->angleLabel->setText(QString::number(_angleValue));
    ui->speedLabel->setText(QString::number(_speedValue));
    ui->tempLabel->setText(QString::number(_tempValue));
    ui->currentLabel->setText(QString::number(_currentValue));
}

void MotorInfo::on_angleLineEdit_editingFinished()
{
    // 创建一个motorTarget消息
    ihb_msgs::msg::ServoMotorJointTarget motorTarget;
    motorTarget.id = _id;
    motorTarget.angle = ui->angleLineEdit->text().toInt();
    motorTarget.speed_limit = ui->speedLineEdit->text().toInt();
    // 获取全局信号发送器
    SignalTransmitter *signal_transmitter = SignalTransmitter::instance();
    // 发送消息
    emit signal_transmitter->ihbServoMotorTargetChangedSignal(motorTarget);
}

void MotorInfo::on_dial_valueChanged(int value)
{
    // info
    qDebug() << "dial value changed";
    // 创建一个motorTarget消息
    ihb_msgs::msg::ServoMotorJointTarget motorTarget;
    motorTarget.id = _id;
    motorTarget.angle = ui->dial->value();
    motorTarget.speed_limit = ui->speedLineEdit->text().toInt();
    // 获取全局信号发送器
    SignalTransmitter *signal_transmitter = SignalTransmitter::instance();
    // 发送消息
    emit signal_transmitter->ihbServoMotorTargetChangedSignal(motorTarget);
}
