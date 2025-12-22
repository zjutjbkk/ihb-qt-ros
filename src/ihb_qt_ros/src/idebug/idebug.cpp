//idebug.cpp
#include "idebug.h"
#include "ui_idebug.h"
#include <QTabBar>
#include "itvch.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QWidget>
#include <QKeyEvent>
#include <QFileDialog>
#include "src/connect/signaltransmitter.h"
#include "commandtextedit.h"
#include <QIntValidator>
#include <QSettings>
// setting
#include <QSettings>
#include <QTimer>
#include <QPainter>
#include <QLineEdit>
// 添加QCustomPlot的包含
#include "qcustomplot.h"
#include "forcesensor.h"
#include "ihb_msgs/msg/ihb_status.hpp"
#include "ihb_msgs/msg/stepper_motor_joint_status.hpp"
#include "ihb_msgs/msg/servo_motor_joint_status.hpp"
#include "ihb_msgs/msg/muscle_joint_status.hpp"
#include "rclcpp/rclcpp.hpp"
#include "ihb_msgs/msg/stepper_motor_joint_target.hpp"
#include <QElapsedTimer>
#include <QTime>
// #include "src/idebug/musclejoint.h"
// #include "src/qcustomplot/qcustomplot.h"

IDebug::IDebug(QWidget *parent) : QWidget(parent),
                                  ui(new Ui::IDebug)
{
    ui->setupUi(this);
    ui->IDebugTab->tabBar()->setObjectName("IDebugTabBar");

    //    ui->actionTableWidget->setParent(this);

    //    QFile qss(":/files/idebug.qss");
    //    qss.open(QFile::ReadOnly);
    //    this->setStyleSheet(qss.readAll());
    //    setITVNum(290);
    layout = new QGridLayout(ui->Itvs);

    //    serialDataTx = new QString();
    //    serialDataRx = new QString();
    // 默认itv数量24
    ui->lineEditItvNum->setText("24");
    setITVNum(24);

    // 限制延时只能输入数字
    QIntValidator *validatorlineEditItvNum = new QIntValidator(ui->lineEditItvNum);
    ui->lineEditItvNum->setValidator(validatorlineEditItvNum);

    QIntValidator *validatorcommandTime = new QIntValidator(ui->commandTime);
    ui->commandTime->setValidator(validatorcommandTime);

    // 设置serialData
    //    QString *data = new QString("A))A01B2345ccAccA02B2014AA");
    //   //  qDebug() << "---------------------";
    //    serialDataRxChanged(data);

    //  //  qDebug() << "here" << serialDataTx << *serialDataTx;
    ui->IDebugTab->setCurrentIndex(0);

    //.........定时检查电机状态并表示在teach中的label........................................
    motorStatusTimer = new QTimer(this);
        //通过 qobject_cast 将 ui->widget_2 转换为 teach*。
        //确保 widget_2 的 objectName 在 UI Designer 中设置为 "widget_2"，且其类为 teach。
        //如果转换成功，teachWidget 将指向已存在的 teach 实例；否则，将输出调试信息。
    teach *teachWidget = qobject_cast<teach*>(ui->widget_2);
    if (teachWidget) {
        qDebug() << "Successfully cast widget_2 to teach.";
    } else {
        qDebug() << "Failed to cast widget_2 to teach.";
    }

    // 串口数据接收 修改itvch的值
    connect(signalTransmitter, &SignalTransmitter::SerialRxDataChangedSignal, this, [=](QString data)
            {
         // 判断串口是否连接
        // qDebug() << "*serialDataRx" << *serialDataRx;
        itvRxData.append(data);
        QString pattern = "A(\\d{2})B(\\d{4})";
        QRegExp rx(pattern);

        int pos = 0;
        while ((pos = rx.indexIn(itvRxData, pos)) != -1)
        {
            int channelValue = rx.cap(1).toInt();
            int dataValue = rx.cap(2).toInt();

          //  qDebug() << "通道" << channelValue << "的值为" << dataValue;
            // 设置对象的值
            ITVCH *itvch = this->findChild<ITVCH *>(QString("itvch%1").arg(channelValue));
            // 判断是否找到对象
            if (itvch != NULL)
            {

                itvch->update(dataValue);
            }

            //删除已经解析过的子串
            itvRxData.remove(pos, rx.matchedLength());
        } });
    connect(ui->commandText, &CommandTextEdit::finishRunSignal, this, [=]()
            { ui->actionStartButton->setText("开始"); });

   

    // ihb state
      // 你需要使用qRegisterMetaType函数来注册这个类型。
    connect(signalTransmitter, &SignalTransmitter::IhbStatusChangedSignal, this, [=](ihb_msgs::msg::IhbStatus ihb_status)
            {
                //log 
                // qDebug() << "ihb_status callback" ;
                ihb_status_callback(ihb_status);
            });


    if (!QFile::exists("./config.ini"))
    {
        // 创建配置文件实例
        QSettings *configIniWrite = new QSettings("./config.ini", QSettings::IniFormat);
        // 将信息写入配置文件
        configIniWrite->setValue("IHB/itvNum", 24);
        setITVNum(24);
        configIniWrite->setValue("IHB/command", "A88B0000");
        // configIniWrite->setValue("config/circle", ui->circleCheckBox->isChecked());
        // configIniWrite->setValue("config/commandTime", ui->commandTime->text());
        // configIniWrite->setValue("config/skipZero", ui->skipZeroCheck->isChecked());
        // 保存配置
        configIniWrite->sync();
        // 写入完成后删除指针
        delete configIniWrite;
    }
    else
    {
        // 创建配置文件读取实例
        QSettings *configIniRead = new QSettings("./config.ini", QSettings::IniFormat);
        // 将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
        int settingItvNum = configIniRead->value("IHB/itvNum").toInt();
        QString settingCommand = configIniRead->value("IHB/command").toString();
        // 判断输入是否合法 > 0  && < 200
        if (settingItvNum <= 0 || settingItvNum > 200)
        {
            // 如果不合法，改为默认值
            settingItvNum = 24;
            //            setITVNum(24);
        }
        setITVNum(settingItvNum);
        // 判断settingCommand是否合法
        if (settingCommand.length() == 0)
        {
            // 如果不合法，改为默认值
            settingCommand = "A88B0000";
        }
        // QString circle = configIniRead->value("config/circle").toString();
        // QString commandTime = configIniRead->value("config/commandTime").toString();
        // QString skipZero = configIniRead->value("config/skipZero").toString();
        // 将ini文件关闭
        delete configIniRead;
        // 设置默认值
        ui->lineEditItvNum->setText(QString::number(settingItvNum));
        ui->commandText->setPlainText(settingCommand);
        // ui->circleCheckBox->setChecked(circle);
        // ui->commandTime->setText(commandTime);
        // ui->skipZeroCheck->setChecked(skipZero);
    }
}

IDebug::~IDebug()
{
    // 保存配置文件
    // 创建配置文件实例
    QSettings *configIniWrite = new QSettings("./config.ini", QSettings::IniFormat);
    // 将信息写入配置文件
    configIniWrite->setValue("IHB/itvNum", ui->lineEditItvNum->text());
    configIniWrite->setValue("IHB/command", ui->commandText->toPlainText());
    // configIniWrite->setValue("config/circle", ui->circleCheckBox->isChecked());
    // configIniWrite->setValue("config/commandTime", ui->commandTime->text());
    // configIniWrite->setValue("config/skipZero", ui->skipZeroCheck->isChecked());
    // 遍历layout内的所有itvch

    configIniWrite->sync();
    // 写入完成后删除指针
    delete configIniWrite;
    delete ui;
}
void IDebug::ihb_status_callback(ihb_msgs::msg::IhbStatus msg){
    // PID调试界面会自己订阅 IhbStatusChangedSignal,这里不需要处理
    // 如果有其他需要处理的逻辑,可以在这里添加
}

void IDebug::realtimeDataSlot()
{
    // 不做任何事情，PID调试界面会自己处理
}

void IDebug::setITVNum(int numITVCH)
{

    // 注释中的代码有问题，不能用，会导致程序崩溃，已经修复，留着学习
    // // 清空layout 内的所有
    // QLayoutItem *child;
    // while ((child = layout->takeAt(0)) != 0) {
    //   //  qDebug() << "delete child" << child->widget();
    //     delete child->widget();
    //     delete child;
    // }
    // // 清空layout
    // scoll content  itvs
    // 设置content grid布局

    // int row, col;
    // for (int i = 0; i < numITVCH; i++) {
    //     row = i / 4;
    //     col = i % 4;
    //     ITVCH* itvch = new ITVCH();
    //     layout->addWidget(itvch, row, col);
    //   //  qDebug() << "row: " << row << "col: " << col;
    // }
    // ITVCH *itvch1 = new ITVCH(this, 1, 0);
    // 崩溃原因
    // 当鼠标点击layout内的元素时，会再次触发setITVNum
    // 但是当前点击的对象实际已经被删除，导致程序崩溃
    // MuscleJoint *tetee = new MuscleJoint(this);
    // 判断输入是否合法 > 0  && < 200
    if (numITVCH <= 0 || numITVCH > 200)
    {
        // 如果不合法，弹窗
        QMessageBox::warning(this, "警告", "请输入1-200之间的数字");
        return;
    }

    // 获取当前的itv数量
    int currentNum = layout->count();
    //  //  qDebug() << "currentNum: " << currentNum;
    // 如果当前的itv数量大于要设置的itv数量
    if (currentNum > numITVCH)
    {
        // 删除多余的itv
        for (int i = currentNum - 1; i >= numITVCH; i--)
        {
            QLayoutItem *child = layout->takeAt(i);
            //          //  qDebug() << "delete child" << child->widget();
            delete child->widget();
            delete child;
        }
    }
    else if (currentNum < numITVCH)
    {
        // 如果当前的itv数量小于要设置的itv数量
        // 添加itv
        int row, col;
        for (int i = currentNum; i < numITVCH; i++)
        {
            row = i / 4;
            col = i % 4;
            //          //  qDebug() << "rows: " << serialDataTx;
            ITVCH *itvch = new ITVCH(this, i, 0);
            // 设置itvch的objectName

            //          //  qDebug() << "itvch objectName: " << itvch->objectName();
            layout->addWidget(itvch, row, col);
            //          //  qDebug() << "row: " << row << "col: " << col;
            //            connect(itvch,SIGNAL(dataChangeSignal()),this,SLOT(log()));
            //            connect(itvch, SIGNAL(dataChangeSignal()), this, SLOT(serialDataChangedSlot()));
        }
    }
}
// void IDebug::log()
//{
//     // qDebug() << *serialDataTx;
// }
//  void IDebug::serialDataChangedSlot()
//  {
//      emit serialDataTxChangedSignal(serialDataTx);
//  }
void IDebug::on_lineEditItvNum_editingFinished()
{
    //    if (ui->lineEditItvNum->text())
    // 判断输入是否合法
    if (ui->lineEditItvNum->text().toInt() > 0)
    {
        // 设置itv数量
        setITVNum(ui->lineEditItvNum->text().toInt());
    }
}
// void IDebug::serialDataRxChanged(QString *serialDataRx)
// {
//     // serialDataTx 数据为 A00B1234A01B1234R
//     // 解析字符串
//     // 使用正则表达式匹配数据
//   //  qDebug() << "*serialDataRx" << *serialDataRx;
//     QString pattern = "A(\\d{2})B(\\d{4})";
//     QRegExp rx(pattern);

//     int pos = 0;
//     while ((pos = rx.indexIn(*serialDataRx, pos)) != -1)
//     {
//         int channelValue = rx.cap(1).toInt();
//         int dataValue = rx.cap(2).toInt();

//       //  qDebug() << "通道" << channelValue << "的值为" << dataValue;
//         // 设置对象的值
//         ITVCH *itvch = this->findChild<ITVCH *>(QString("itvch%1").arg(channelValue));
//         // 判断是否找到对象
//         if (itvch != NULL)
//         {

//             itvch->update(dataValue);
//         }

//         // 删除已经解析过的子串
//         serialDataRx->remove(pos, rx.matchedLength());
//     }
// }
void IDebug::on_pushButton_clicked()
{
    // serialDataTx->append("A88B0000");
    // emit serialDataTxChangedSignal(serialDataTx);
    emit signalTransmitter->SerialTxDataChangedSignal("A88B0000");
    // 遍历layout内的所有itvch
    int currentNum = layout->count();
    for (int i = 0; i < currentNum; i++)
    {
        ITVCH *itvch = this->findChild<ITVCH *>(QString("itvch%1").arg(i));
        // 判断是否找到对象
        if (itvch != NULL)
        {

            itvch->update(0);
        }
    }
    ui->commandText->stop();
}

void IDebug::on_actionStartButton_clicked()
{

    // ui->actionTableWidget->start(ui->circleCheckBox->isChecked());
    // 判断按钮文字
    if (ui->actionStartButton->text() == "开始")
    {
        // 如果是开始，设置为停止
        ui->actionStartButton->setText("停止");
        // 开始发送数据
        ui->commandText->start(ui->circleCheckBox->isChecked(), ui->commandTime->text().toInt());
    }
    else
    {
        // 如果是停止，设置为开始
        ui->actionStartButton->setText("开始");
        // 停止发送数据
        ui->commandText->stop();
    }
}

void IDebug::on_updateStatus_clicked()
{
    emit signalTransmitter->SerialTxDataChangedSignal("A99B0000");
}

void IDebug::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_A: // 弹出关于界面
        qDebug("a");
        break;
        //    case Qt::Key_Escape:
        //        this->hide();
        //        this->close();
        // 空格急停
    case Qt::Key_Space:
        on_pushButton_clicked();
        break;
    // u 更新状态
    case Qt::Key_U:
        emit signalTransmitter->SerialTxDataChangedSignal("A99B0000");
        break;
    default:
        break;
    }
}

void IDebug::on_helpButton_clicked()
{
    //     这里是命令列表
    // D100   // 延时100ms
    // A00B1000 // 通道0设置为1000
    // A88B0000 // 全部置零
    // A99B0000 // 读取ADC
    // 附加文本到文本框 plain text edit
    ui->commandText->appendPlainText("命令以行分隔 \n D100   // 延时100ms\nA00B1000 // 通道0设置为1000\nA88B0000 // 全部置零\n A99B0000 // 读取ADC  \n支持读取和保存为txt文件");
}

void IDebug::on_readButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择文本文件"), "", tr("文本文件 (*.txt)"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            //            QString fileContent = in.readAll();
            ui->commandText->setPlainText(in.readAll());
            file.close();
        }
    }
}

void IDebug::on_saveButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存文件"), "", tr("文本文件 (*.txt)"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            QString fileContent = ui->commandText->toPlainText(); // 假设你要保存的文件内容存在fileContent变量中
            out << fileContent;
            file.close();
        }
    }
}

void IDebug::on_clearButton_clicked()
{
    ui->commandText->setPlainText("");
}

void IDebug::on_recordButton_clicked()
{
    // 要添加的字符串 初始为空
    QString recordData = "";
    // 遍历layout内的所有itvch
    int currentNum = layout->count();
    for (int i = 0; i < currentNum; i++)
    {
        ITVCH *itvch = this->findChild<ITVCH *>(QString("itvch%1").arg(i));
        // 判断是否找到对象
        if (itvch != NULL)
        {
            // 判断itvch的值是否为0
            if (itvch->chValue > 0 || !ui->skipZeroCheck->isChecked())
            {
                // 如果不为0，添加到recordData 格式化为 A00B0000
                recordData.append(QString("A%1B%2").arg(itvch->chNum, 2, 10, QChar('0')).arg(itvch->chValue, 4, 10, QChar('0')));
            }
        }
    }
    // 将recordData 添加到commandText
    ui->commandText->appendPlainText(recordData);
}
