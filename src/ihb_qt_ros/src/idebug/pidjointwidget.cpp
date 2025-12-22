#include "pidjointwidget.h"
#include "ui_pidjointwidget.h"
#include <QDebug>

PIDJointWidget::PIDJointWidget(int jointId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PIDJointWidget),
    m_jointId(jointId),
    m_angle0(0.0),
    m_angle1(0.0),
    m_targetAngle0(0.0),
    m_targetAngle1(0.0)
{
    ui->setupUi(this);

    // 设置关节ID标题
    ui->groupBox_joint->setTitle(QString("关节 %1 (Muscle Joint %1)").arg(jointId));

    // 连接参数变更信号
    connect(ui->doubleSpinBox_kp, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PIDJointWidget::onKpChanged);
    connect(ui->doubleSpinBox_ki, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PIDJointWidget::onKiChanged);
    connect(ui->doubleSpinBox_kd, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PIDJointWidget::onKdChanged);

    // 连接目标角度变更信号
    connect(ui->doubleSpinBox_target_angle0, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PIDJointWidget::onAngle0Changed);
    connect(ui->doubleSpinBox_target_angle1, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PIDJointWidget::onAngle1Changed);

    // 连接微调按钮
    connect(ui->pushButton_kp_inc, &QPushButton::clicked, this, &PIDJointWidget::onKpIncrement);
    connect(ui->pushButton_kp_dec, &QPushButton::clicked, this, &PIDJointWidget::onKpDecrement);
    connect(ui->pushButton_ki_inc, &QPushButton::clicked, this, &PIDJointWidget::onKiIncrement);
    connect(ui->pushButton_ki_dec, &QPushButton::clicked, this, &PIDJointWidget::onKiDecrement);
    connect(ui->pushButton_kd_inc, &QPushButton::clicked, this, &PIDJointWidget::onKdIncrement);
    connect(ui->pushButton_kd_dec, &QPushButton::clicked, this, &PIDJointWidget::onKdDecrement);

    // 连接发送按钮
    connect(ui->pushButton_send, &QPushButton::clicked, this, &PIDJointWidget::onSendButtonClicked);
}

PIDJointWidget::~PIDJointWidget()
{
    delete ui;
}

void PIDJointWidget::setParams(double kp, double ki, double kd)
{
    ui->doubleSpinBox_kp->setValue(kp);
    ui->doubleSpinBox_ki->setValue(ki);
    ui->doubleSpinBox_kd->setValue(kd);
}

void PIDJointWidget::getParams(double &kp, double &ki, double &kd) const
{
    kp = ui->doubleSpinBox_kp->value();
    ki = ui->doubleSpinBox_ki->value();
    kd = ui->doubleSpinBox_kd->value();
}

void PIDJointWidget::setTargetAngles(double angle0, double angle1)
{
    ui->doubleSpinBox_target_angle0->setValue(angle0);
    ui->doubleSpinBox_target_angle1->setValue(angle1);
    m_targetAngle0 = angle0;
    m_targetAngle1 = angle1;
}

void PIDJointWidget::getTargetAngles(double &angle0, double &angle1) const
{
    angle0 = ui->doubleSpinBox_target_angle0->value();
    angle1 = ui->doubleSpinBox_target_angle1->value();
}

void PIDJointWidget::updateStatus(const ihb_msgs::msg::MuscleJointStatus &status)
{
    // 更新两个角度值
    m_angle0 = status.angle[0];
    m_angle1 = status.angle[1];

    // 更新UI显示
    ui->label_angle0_value->setText(QString("%1°").arg(m_angle0, 0, 'f', 1));
    ui->label_angle1_value->setText(QString("%1°").arg(m_angle1, 0, 'f', 1));
}

void PIDJointWidget::onKpChanged(double value)
{
    emit paramsChanged(m_jointId, value, ui->doubleSpinBox_ki->value(), ui->doubleSpinBox_kd->value());
}

void PIDJointWidget::onKiChanged(double value)
{
    emit paramsChanged(m_jointId, ui->doubleSpinBox_kp->value(), value, ui->doubleSpinBox_kd->value());
}

void PIDJointWidget::onKdChanged(double value)
{
    emit paramsChanged(m_jointId, ui->doubleSpinBox_kp->value(), ui->doubleSpinBox_ki->value(), value);
}

void PIDJointWidget::onAngle0Changed(double value)
{
    m_targetAngle0 = value;
    emit targetAnglesChanged(m_jointId, value, m_targetAngle1);
}

void PIDJointWidget::onAngle1Changed(double value)
{
    m_targetAngle1 = value;
    emit targetAnglesChanged(m_jointId, m_targetAngle0, value);
}

void PIDJointWidget::onSendButtonClicked()
{
    emit sendParamsClicked(m_jointId);
}

void PIDJointWidget::onKpIncrement()
{
    ui->doubleSpinBox_kp->setValue(ui->doubleSpinBox_kp->value() + ui->doubleSpinBox_kp->singleStep());
}

void PIDJointWidget::onKpDecrement()
{
    ui->doubleSpinBox_kp->setValue(ui->doubleSpinBox_kp->value() - ui->doubleSpinBox_kp->singleStep());
}

void PIDJointWidget::onKiIncrement()
{
    ui->doubleSpinBox_ki->setValue(ui->doubleSpinBox_ki->value() + ui->doubleSpinBox_ki->singleStep());
}

void PIDJointWidget::onKiDecrement()
{
    ui->doubleSpinBox_ki->setValue(ui->doubleSpinBox_ki->value() - ui->doubleSpinBox_ki->singleStep());
}

void PIDJointWidget::onKdIncrement()
{
    ui->doubleSpinBox_kd->setValue(ui->doubleSpinBox_kd->value() + ui->doubleSpinBox_kd->singleStep());
}

void PIDJointWidget::onKdDecrement()
{
    ui->doubleSpinBox_kd->setValue(ui->doubleSpinBox_kd->value() - ui->doubleSpinBox_kd->singleStep());
}

