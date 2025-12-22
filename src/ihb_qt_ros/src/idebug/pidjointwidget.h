#ifndef PIDJOINTWIDGET_H
#define PIDJOINTWIDGET_H

#include <QWidget>
#include "ihb_msgs/msg/muscle_joint_status.hpp"

namespace Ui {
class PIDJointWidget;
}

class PIDJointWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PIDJointWidget(int jointId, QWidget *parent = nullptr);
    ~PIDJointWidget();

    // 设置/获取 PID 参数
    void setParams(double kp, double ki, double kd);
    void getParams(double &kp, double &ki, double &kd) const;

    // 设置/获取目标角度
    void setTargetAngles(double angle0, double angle1);
    void getTargetAngles(double &angle0, double &angle1) const;

    // 更新状态显示
    void updateStatus(const ihb_msgs::msg::MuscleJointStatus &status);

signals:
    void paramsChanged(int jointId, double kp, double ki, double kd);
    void sendParamsClicked(int jointId);
    void targetAnglesChanged(int jointId, double angle0, double angle1);

private slots:
    void onKpChanged(double value);
    void onKiChanged(double value);
    void onKdChanged(double value);
    void onAngle0Changed(double value);
    void onAngle1Changed(double value);
    void onSendButtonClicked();
    void onKpIncrement();
    void onKpDecrement();
    void onKiIncrement();
    void onKiDecrement();
    void onKdIncrement();
    void onKdDecrement();

private:
    Ui::PIDJointWidget *ui;
    int m_jointId;
    double m_angle0;  // 当前 angle[0] (从状态反馈获取)
    double m_angle1;  // 当前 angle[1] (从状态反馈获取)
    double m_targetAngle0;  // 目标 angle[0] (用户设置)
    double m_targetAngle1;  // 目标 angle[1] (用户设置)
};

#endif // PIDJOINTWIDGET_H

