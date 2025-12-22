#include "commandtextedit.h"
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QWidget>
#include "src/connect/signaltransmitter.h"
#include <QMessageBox>

#include <QRegularExpression>
CommandTextEdit::CommandTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    // 设置编码格式 utf-8
    //     QTextCodec *codec = QTextCodec::codecForName("utf-8");
    //     QTextCodec::setCodecForLocale(codec);
    //     QTextCodec::setCodecForCStrings(codec);
}

void CommandTextEdit::start(bool circle, int time)
{

    // 判断输入是否合法
    QRegularExpression regex("D\\d+"); // D followed by zero or more digits
                                       //  //  qDebug()<<"regex"<<regex.match(toPlainText()).hasMatch();

    if (circle && (time == 0) && !regex.match(toPlainText()).hasMatch())
    {
        // 循环至少需 要一个延时函数  避免死循环
        QMessageBox::critical(nullptr, "Error", "循环至少需要一个延时");
        emit finishRunSignal();
        return;
    }
    __circle = circle;
    __stop = false;
    __time = time;
    currentActionRow = 0;
    runItem();
}
void CommandTextEdit::stop()
{
    __stop = true;

    emit finishRunSignal();
    //    runItem();
}
void CommandTextEdit::runItem()
{
    qDebug() << "time1" << __time;
    //  qDebug() << currentActionRow;
    if (__stop)
    {
        __time = 0;
        emit finishRunSignal();
        return;
    }
    // 急停

    if (currentActionRow == toPlainText().split("\n").length())
    { // 已经完成最后一行
        currentActionRow = 0;
        if (!__circle)
        {
            emit finishRunSignal();
            return;
        }
    }

    // 定时发送功能
    if (__time > 0 && !__wait)
    {
        // 等待
        QTimer::singleShot(__time, this, [&]()
                           {
//          //  qDebug() << "time1" << time;
            runItem(); });
        __wait = true;
        return;
    }
    else
    {
        __wait = false;
    }

    //  qDebug() << "len" << toPlainText().split("\n").at(currentActionRow).length();

    if (toPlainText().split("\n").at(currentActionRow).length() > 2)
    { // 判断当前行内容是否为空
        QString data = toPlainText().split("\n").at(currentActionRow);
        //  qDebug() << data << toPlainText().split("\n").length();
        if (data.at(0) == 'D')
        { // 延时 不定时发送
            // 判断延时时间是否合法
            if (data.mid(1).toInt() > 0)
            {
                //  qDebug() << "delay:" << data.midRef(1).toInt();
                // 创建单次定时器延时
                QTimer::singleShot(data.midRef(1).toInt(), this, [&]()
                                   {
//                  //  qDebug() << "time2" << time;
                    runItem(); });
                // runItem();  //递归
                currentActionRow++;
                return;
            }
            else
            {
                //  qDebug() << "delay:" << data.mid(1).toInt();
                //  qDebug() << "delay time error";
                // 弹窗提示延时时间错误
                QMessageBox::warning(this, "warning", "delay time error");
                emit finishRunSignal();
                return;
            }
        }
        else
        {
            //  qDebug() << "send:" <<  data;
            //            // 发送数据
            sendData(data);
            currentActionRow++;
        }
    }
    else
    {
        currentActionRow++;
    }
    runItem();
}

void CommandTextEdit::sendData(QString indata)
{
    // 判断数据长度
    if (indata.length() < 1)
    {

        QMessageBox::warning(this, "warning", "send data error");
        return;
    }
    // 发送数据
    SignalTransmitter *signalTransmitter = SignalTransmitter::instance();
    if (signalTransmitter->serialState)
        emit signalTransmitter->SerialTxDataChangedSignal(indata);
    else
    {
        QMessageBox::warning(this, "warning", "串口未连接");
        emit finishRunSignal();
        stop();
    }
}
