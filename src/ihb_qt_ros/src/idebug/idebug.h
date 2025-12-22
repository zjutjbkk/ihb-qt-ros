#ifndef IDEBUG_H
#define IDEBUG_H

#include <QWidget>
#include <QGridLayout>
#include <QSharedPointer>
#include <QWidget>
#include <QKeyEvent>
#include <QTimer>
#include  "src/connect/signaltransmitter.h"
//
#include "teach.h" // 包含 teach 的头文件

namespace Ui
{
    class IDebug;
}
class teach;

class IDebug : public QWidget
{
    Q_OBJECT

public:
    explicit IDebug(QWidget *parent = nullptr);
    ~IDebug();


private slots:
    void on_lineEditItvNum_editingFinished();

    void on_pushButton_clicked();

    void on_actionStartButton_clicked();

    void on_updateStatus_clicked();

    void on_helpButton_clicked();

    void on_readButton_clicked();

    void on_saveButton_clicked();

    void on_clearButton_clicked();

//    void on_pushButton_2_clicked();

    void on_recordButton_clicked();

    void realtimeDataSlot();
    void ihb_status_callback(ihb_msgs::msg::IhbStatus msg);

private:
    SignalTransmitter* signalTransmitter = SignalTransmitter::instance();
    Ui::IDebug *ui;
    void setITVNum(int mum);
    QGridLayout *layout;\
    void keyPressEvent(QKeyEvent *event);
    QString itvRxData;
    QTimer dataTimer;
    QTimer *motorStatusTimer;
};

#endif // IDEBUG_H
