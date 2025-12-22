#include "itvch.h"
#include "ui_itvch.h"
#include <QDebug>
#include "src/connect/signaltransmitter.h"

#include "QSignalTransition"
#include <QFile>
#include <QSettings>
ITVCH::ITVCH(QWidget *parent, int Num, int Value) :
    QWidget(parent),
    ui(new Ui::ITVCH)
{
    ui->setupUi(this);
    setObjectName(QString("itvch%1").arg(Num));
    // 绑定chdata
//    chData = Data;
//  //  qDebug()<< chData << Data << *chData;
    //设置chNum
    this->chNum = Num;
    ui->chNum->setText(QString::number(chNum));
    //设置chValue
    this->chValue = Value;

    //设置chValue SpinBox 数字范围
    ui->chValue->setRange(0, 9999);

    ui->chValue->setValue(chValue);
    //设置滑动条
    ui->horizontalSliderValue->setRange(0, 9999);
    ui->horizontalSliderValue->setValue(chValue);
    //设置chData
    // sendData();
    //设置输入框滚轮每次滚动增加100
    
    ui->chValue->setSingleStep(100);
    // 设置滑动条每次滑动的值
    ui->horizontalSliderValue->setSingleStep(100);
    // 设置滚轮每次滚动的滑动条的值


    //判断是否有配置文件
    // 如果没有配置文件则创建配置文件
    // 判断配置文件中是否有chNum的值
    // 如果有则读取配置文件中的值
    // 如果没有则写入默认值
    // 读取配置文件

//    if(QFile::exists("config.ini")){ //如果文件存在
//            // qDebug() << "文件存在";
//        QSettings *configIniRead = new QSettings("config.ini", QSettings::IniFormat);
//        //将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
//        int settingCh = configIniRead->value("IHB/"+objectName()).toInt();
//        if (settingCh == 0) {
//            // 如果没有chNum的值则写入默认值
//            configIniRead->setValue("IHB/"+objectName(), chNum);
//        } else {
//            // 如果有chNum的值则读取配置文件中的值
//            chNum = settingCh;
//        }
//        configIniRead->sync();
//        //读入入完成后删除指针
//        delete configIniRead;
//    } else {
//        // 如果没有配置文件则创建配置文件
//        // 判断配置文件中是否有chNum的值
//        // 如果有则读取配置文件中的值
//        // 如果没有则写入默认值
//        // 读取配置文件
//        QSettings *configIniWrite = new QSettings("config.ini", QSettings::IniFormat);
//        configIniWrite->setValue("IHB/"+objectName(), chNum);
//        configIniWrite->sync();
//        //读入入完成后删除指针
//        delete configIniWrite;
//    }
//    update(chNum,0);



}

ITVCH::~ITVCH()
{
    // 写入配置文件
//    QSettings *configIniWrite = new QSettings("config.ini", QSettings::IniFormat);
//    configIniWrite->setValue("IHB/"+objectName(), chNum);
//    configIniWrite->sync();
    //读入入完成后删除指针
//    delete configIniWrite;
    delete ui;
}

void ITVCH::on_chNum_textChanged(const QString &arg1)
{
//  //  qDebug() << "chNum_change";

}


void ITVCH::update( int Num, int Value)
{

    chNum = Num;
    chValue = Value;

    ui->chNum->setText(QString::number(chNum));
    ui->chValue->setValue(Value);
    //修改滑动条
    ui->horizontalSliderValue->setValue(chValue);
    sendData();

}

void ITVCH::setCh(int ch){
    update(ch,0);
}

void ITVCH::update(int Value){
    chValue = Value;

    ui->chNum->setText(QString::number(chNum));
    ui->chValue->setValue(Value);
    //修改滑动条
    ui->horizontalSliderValue->setValue(chValue);
    sendData();
}

void ITVCH::sendData()
{

    QString *chData = new QString();
  //  qDebug() << "ch send data start";
    // 通信协议 A 01 B 0000
    // 01 为chNum 0000 为chValue
    // 拼接字符串并写入chData
    chData->append("A");
    // chNum 2位
    if (chNum < 10) {
        chData->append("0");
    }
    chData->append(QString::number(chNum));
    chData->append("B");
    // chValue 4位
    if (chValue < 10) {
        chData->append("000");
    } else if (chValue < 100) {
        chData->append("00");
    } else if (chValue < 1000) {
        chData->append("0");
    }
    chData->append(QString::number(chValue));

  //  qDebug() << "ch send data ing" << chValue;
    // 在chData中写入数据
    SignalTransmitter* signalTransmitter = SignalTransmitter::instance();
    emit signalTransmitter->SerialTxDataChangedSignal(*chData);
    // emit dataChangeSignal();
//  //  qDebug() << "ch send data";
    
}
void ITVCH::on_chValue_editingFinished()
{
//    if (ui->chValue->text().toInt() != chValue)
    update(ui->chNum->text().toInt(), ui->chValue->text().toInt());
}


//void ITVCH::on_horizontalSliderValue_rangeChanged(int min, int max)
//{
//  //  qDebug() << "ch slider range change";
//    update(ui->chNum->text().toInt(), ui->horizontalSliderValue->value());
//}


void ITVCH::on_chNum_editingFinished()
{
    //
  //  qDebug() << "ch num change";
}


void ITVCH::on_horizontalSliderValue_valueChanged(int value)
{
    update(ui->chNum->text().toInt(), ui->horizontalSliderValue->value());
//  //  qDebug() <<  "两个value" <<value << ui->horizontalSliderValue->value();
}

