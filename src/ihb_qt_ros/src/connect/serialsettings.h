#ifndef SERIALSETTINGS_H
#define SERIALSETTINGS_H

#include <QWidget>
#include <QSerialPort>


namespace Ui {
class SerialSettings;
}

class QIntValidator;

class SerialSettings : public QWidget
{
    Q_OBJECT
    // Q_PROPERTY(QString rxData READ readSerialData WRITE setRxData  NOTIFY rxDataChanged)
    // Q_PROPERTY(QString txData READ getTxData WRITE sendSerialData NOTIFY txDataChanged)

public:
    struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        bool localEchoEnabled;
    };
    explicit SerialSettings(QWidget *parent = nullptr);
    ~SerialSettings();

    Settings settings() const;

    void sendData(QString data);
    QSerialPort *m_serial = nullptr;

//     //rxData
//     QString readSerialData(); //其他类调用 获取串口接收数据
    

//     //txData
//     bool sendSerialData(QString data); //其他类调用 发送串口数据
    

// signals:
//     void rxDataChanged(QString Data);
//     void txDataChanged(QString Data);

private:

    Ui::SerialSettings *ui;
    Settings m_currentSettings;
    QIntValidator *m_intValidator = nullptr;
    void fillPortsParameters();
    void fillPortsInfo();
    void updateSettings();

    // QString __rxdata;
    // QString __txdata;

    // // 这两个其实可以不要，但或许可以便于理解
    // void setRxData(QString data); //本类调用 设置串口接收数据
    // QString getTxData(); //本类调用 获取串口发送数据
    bool warnned = false;

private slots:
    void showPortInfo(int idx);
    void apply();
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);

     void on_pushButton_clicked();
//     void on_serialPortInfoListBox_activated(int index);


     void on_bluetoothButton_clicked();
};

#endif // SERIALSETTINGS_H
