#include "serialsettings.h"
#include "ui_serialsettings.h"
#include <QMessageBox>
#include <QIntValidator>
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QException>
#include <QDebug>
#include "signaltransmitter.h"
#include <QProcess>
#include <QTimer>
static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SerialSettings::SerialSettings(QWidget *parent) : QWidget(parent),
                                                  ui(new Ui::SerialSettings),
                                                  m_intValidator(new QIntValidator(0, 4000000, this))
{
    ui->setupUi(this);

    ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);
    connect(ui->applyButton, &QPushButton::clicked,
            this, &SerialSettings::apply);
    connect(ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SerialSettings::showPortInfo);
    connect(ui->baudRateBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SerialSettings::checkCustomBaudRatePolicy);
    connect(ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SerialSettings::checkCustomDevicePathPolicy);
    // 实例化串口
    m_serial = new QSerialPort(this);
    fillPortsParameters();
    fillPortsInfo();

    updateSettings();
    SignalTransmitter* signalTransmitter = SignalTransmitter::instance();


     //rxData  这里是匿名lambda函数，可以用来替代slot
     connect(m_serial, &QSerialPort::readyRead, this, [=]() {
         QByteArray data = m_serial->readAll();
         QString str = QString::fromLocal8Bit(data);
         emit signalTransmitter->SerialRxDataChangedSignal(str);
     });

     //txData
     connect(signalTransmitter, &SignalTransmitter::SerialTxDataChangedSignal, this, [=](QString data) {
         // 判断串口是否连接
         if (m_serial->isOpen())
         {
             // QString -> Char
             QByteArray ba = data.toLatin1();
             char *d = ba.data();
             // 发送数据
             m_serial->write(d);
             //弹窗提示
             // QMessageBox::information(this, "发送成功", "发送成功");
             //清空serialData
             // data.clear();
         }
         else
         {

             //弹窗提示
             if (!warnned) // 单次弹窗延时
                QTimer::singleShot(300, this, [=]() {
                    QMessageBox::warning(this, "串口未连接", "请先连接串口");
                });

            //  QMessageBox::warning(this, "串口未连接", "请先连接串口");
            warnned = true;
         }
     });
}

SerialSettings::~SerialSettings()
{
    delete ui;
}

SerialSettings::Settings SerialSettings::settings() const
{
    return m_currentSettings;
}

void SerialSettings::sendData(QString data)
{
    // 使用串口发送数据
    //      m_serial->write(data.toWCharArray());
    // 判断串口是否打开
    if (!m_serial->isOpen())
    { // 弹窗
        QMessageBox::critical(this, tr("串口未打开"), m_serial->errorString());
        return;
    }

    QByteArray ba = data.toLatin1();
    char *d = ba.data();
    m_serial->write(d);
}

void SerialSettings::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    const QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();
    ui->descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void SerialSettings::apply()
{
    updateSettings();

    // 设置串口号
    m_serial->setPortName(m_currentSettings.name);
    qDebug() << m_currentSettings.name;
    // 设置波特率
    m_serial->setBaudRate(m_currentSettings.baudRate);
    // 设置数据位数
    m_serial->setDataBits(m_currentSettings.dataBits);
    // 设置奇偶校验
    m_serial->setParity(m_currentSettings.parity);
    // 设置停止位
    m_serial->setStopBits(m_currentSettings.stopBits);
    // 设置流控制
    m_serial->setFlowControl(m_currentSettings.flowControl);
    // 判断当前按钮文字
    if (ui->applyButton->text() == tr("打开串口"))
    {
        // 尝试打开串口
        qDebug() << "open serial port";
        //        try
        //        {
        //            m_serial->open(QIODevice::ReadWrite);
        // qDebug() << m_serial->open(QIODevice::ReadWrite);
        if (!m_serial->open(QIODevice::ReadWrite))
        {
            QMessageBox::critical(this, tr("Error"), m_serial->errorString());
            return;
        }
        // 发送测试数据
        m_serial->write("hello world");
        // 设置按钮文字
        ui->applyButton->setText(tr("关闭串口"));

        //        }
        //        catch (QException e)
        //        {
        //            QMessageBox::critical(this, tr("Error"), m_serial->errorString());
        //        }
    }
    else
    {
        // 尝试关闭串口  这几个try好像没什么用，算了，先留着吧
        qDebug() << "close serial port";
        //        try
        //        {
        m_serial->close();
        // 设置按钮文字
        ui->applyButton->setText(tr("打开串口"));
        warnned = false;
        //        }
        //        catch (QException e)
        //        {
//        QMessageBox::critical(this, tr("Error"), m_serial->errorString());
        //        }
    }
    SignalTransmitter* signalTransmitter = SignalTransmitter::instance();
    signalTransmitter->serialState = true;

    //    hide();
}

void SerialSettings::checkCustomBaudRatePolicy(int idx)
{
    const bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate)
    {
        ui->baudRateBox->clearEditText();
        QLineEdit *edit = ui->baudRateBox->lineEdit();
        edit->setValidator(m_intValidator);
    }
}

void SerialSettings::checkCustomDevicePathPolicy(int idx)
{
    const bool isCustomPath = !ui->serialPortInfoListBox->itemData(idx).isValid();
    ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        ui->serialPortInfoListBox->clearEditText();
}

void SerialSettings::fillPortsParameters()
{
    ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);

    ui->baudRateBox->addItem(tr("Custom"));

    ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBitsBox->setCurrentIndex(3);

    ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}

void SerialSettings::fillPortsInfo()
{
    ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
    {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortInfoListBox->addItem(list.first() + "-" + list.at(1), list);
    }

    ui->serialPortInfoListBox->addItem(tr("Custom"));
}

void SerialSettings::updateSettings()
{
    // m_currentSettings.name = ui->serialPortInfoListBox->currentText();
    // 删除文字后面的描述信息
    m_currentSettings.name = ui->serialPortInfoListBox->currentText().split("-").first();

    if (ui->baudRateBox->currentIndex() == 4)
    {
        m_currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
    }
    else
    {
        m_currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
            ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    }
    m_currentSettings.stringBaudRate = QString::number(m_currentSettings.baudRate);

    m_currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
        ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
    m_currentSettings.stringDataBits = ui->dataBitsBox->currentText();

    m_currentSettings.parity = static_cast<QSerialPort::Parity>(
        ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    m_currentSettings.stringParity = ui->parityBox->currentText();

    m_currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
        ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
    m_currentSettings.stringStopBits = ui->stopBitsBox->currentText();

    m_currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
        ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
    m_currentSettings.stringFlowControl = ui->flowControlBox->currentText();

    //    m_currentSettings.localEchoEnabled = ui->localEchoCheckBox->isChecked();
}

void SerialSettings::on_pushButton_clicked()
{
    fillPortsParameters();
    fillPortsInfo();

    updateSettings();
}

//void SerialSettings::on_serialPortInfoListBox_activated(int index)
//{
//    fillPortsParameters();
//    fillPortsInfo();

//    updateSettings();
//    qDebug("activate");
//}

//QString SerialSettings::readSerialData()
//{
//    return __rxdata;
//}
//void SerialSettings::setRxData(QString data)
//{
//    __rxdata = data;
//    emit rxDataChanged(data);
//}

//// txData
//bool SerialSettings::sendSerialData(QString data)
//{
//    __txdata = data;
//    emit txDataChanged(data);
//}
//QString SerialSettings::getTxData()
//{
//    return __txdata;
//}

void SerialSettings::on_bluetoothButton_clicked()
{
    QString program = "control";
    QStringList arguments;
    arguments << "bthprops.cpl,,6,2";

    QProcess::startDetached(program, arguments);
}

