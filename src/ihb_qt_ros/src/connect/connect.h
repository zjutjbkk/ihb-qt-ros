#ifndef CONNECT_H
#define CONNECT_H

#include <QWidget>

namespace Ui {
class Connect;
}

class Connect : public QWidget
{
    Q_OBJECT




public:
    explicit Connect(QWidget *parent = nullptr);
    ~Connect();
//    QString *serialDataRx = new QString();

//    QString *serialRxData = nullptr;
//    QString *serialTxData = nullptr;

//signals:
//    void rxSerialDataSignal(QString *serialDataRx);

//public slots:
//    void sendSerialData(QString *serialDataTx);
//    void receSerialData();

private:
    Ui::Connect *ui;
};

#endif // CONNECT_H
