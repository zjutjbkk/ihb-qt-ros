#include "forcesensor.h"
#include "ui_forcesensor.h"

ForceSensor::ForceSensor(QWidget *parent,int id) :
  QWidget(parent),
  ui(new Ui::ForceSensor)
{
  ui->setupUi(this);
  ui->idLabel->setText(QString::number(id));
  _id = id;
}

ForceSensor::~ForceSensor()
{
  delete ui;
}
void ForceSensor::update(ihb_msgs::msg::ForceSensorStatus msg){
    // ui->forceLabel->setText(QString::number(data));
    // 拼接字符串
    int data = msg.force;
    QString str = QString::number(_id)+":"+QString::number(data);
    ui->idLabel->setText(str);
    // 设置颜色 根据data的值 int(data/4096.0*255)
    QString color = QString("border-width: 10px; border-color: rgb(%1,100,200); border-style: solid; ").arg(int(data/4096.0*255));
    ui->idLabel->setStyleSheet(color);
    // ui->idLabel->setStyleSheet("color:red");
}