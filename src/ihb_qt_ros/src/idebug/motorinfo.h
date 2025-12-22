//motorinfo.h
#ifndef MOTORINFO_H
#define MOTORINFO_H

#include <QWidget>
#include <QDial>
#include <QDebug>
#include "ihb_msgs/msg/servo_motor_joint_status.hpp"
#include "ihb_msgs/msg/servo_motor_joint_target.hpp"
namespace Ui {
class MotorInfo;
}

class MotorInfo : public QWidget
{
    Q_OBJECT

public:
    explicit MotorInfo(QWidget *parent = nullptr,int id = 0);
    ~MotorInfo();
    QDial *motoDial = nullptr;
    int angleValue;
    int id() const{
        return _id;
    }
    void setId(int id);
    void update(ihb_msgs::msg::ServoMotorJointStatus msg);
    // void updateTarget(ihb_msgs::msg::ServoMotorJointTarget msg);
    //---------------------------------------------------------
    // Getter 方法
    int getAngleValue() const { return _angleValue; }
    int getSpeedValue() const { return _speedValue; }
    int getTempValue() const { return _tempValue; }
    int getCurrentValue() const { return _currentValue; }
    // 新增的启用状态方法
    bool isEnable() const { return _isEnable; }
    //----------------------------------------------------------
signals:

private slots:
//    void valueChangedSlot(QObject* senderObject);
    void on_angleLineEdit_editingFinished();
    void on_dial_valueChanged(int value);

private:
    Ui::MotorInfo *ui;
    int _id;
    int _angleValue;
    int _speedValue;
    int _tempValue;
    int _currentValue;
    // 新增的启用状态成员变量
    bool _isEnable = false;


};

#endif // MOTORINFO_H
