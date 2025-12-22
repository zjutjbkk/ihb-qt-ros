#include "connect.h"
#include "ui_connect.h"
#include <QDebug>
#include <QMessageBox>
Connect::Connect(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Connect)
{
    ui->setupUi(this);


    // 串口数据接收 ddaddawd   dawd
//    connect(ui->serialSetting->m_serial,SIGNAL(readyRead()),this,SLOT(receSerialData()));

}

Connect::~Connect()
{
    delete ui;
}


//void Connect::sendSerialData(QString *serialDataTx){
////    qDebug() << "cnnn" <<serialDataTx << *serialDataTx;
////    // 判断串口是否连接
////    if(ui->serialSetting->m_serial->isOpen())
////    {
////        // QString -> Char
//////        QByteArray ba = serialDataTx->toLatin1();
//////        char *data = ba.data();
//////        // 发送数据
//////        ui->serialSetting->m_serial->write(data);
////        ui->serialSetting->sendData(*serialDataTx);
////        //弹窗提示
////        // QMessageBox::information(this, "发送成功", "发送成功");
////        //清空serialData
////        serialDataTx->clear();
////    }else{
////        //弹窗提示
////        QMessageBox::warning(this, "串口未连接", "请先连接串口");
////    }


//}
//void Connect::receSerialData(){
//    serialDataRx->append(ui->serialSetting->m_serial->readAll());
//    qDebug() << serialDataRx << *serialDataRx;
//    emit rxSerialDataSignal(serialDataRx);
//}
