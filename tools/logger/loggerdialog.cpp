#include "loggerdialog.h"
#include "ui_loggerdialog.h"

#include <QtCore/QThread>
#include <QDebug>

LoggerDialog::LoggerDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoggerDialog)
{
    ui->setupUi(this);

    _serialPort = new SerialPort(this);
    _logParser = new LogParser(_serialPort, this);

    connect(ui->serialPortComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onSerialPortSelected(int)));
    connect(ui->connectButton, SIGNAL(pressed()), SLOT(onConnectButtonPressed()));
    connect(_logParser, SIGNAL(logMessageReceived(QString)), SLOT(onLogMessageReceived(QString)));
    connect(_logParser, SIGNAL(packetReceived(QString)), SLOT(onPacketReceived(QString)));

    QThread readerThread;
    _logParser->moveToThread(&readerThread);
    readerThread.start();

    detectSerialPorts();
}

LoggerDialog::~LoggerDialog()
{
    delete ui;
}

void LoggerDialog::detectSerialPorts()
{
    _serialPorts = SerialPortInfo::availablePorts();
    qDebug() << "Number of serial ports found: " << _serialPorts.count();
    for (int i = 0; i < _serialPorts.count(); i++)
    {
        const SerialPortInfo &info = _serialPorts.at(i);
        QString s(QObject::tr("Port: %1\n"
                              "Location: %2\n"
                              "Description: %3\n"
                              "Manufacturer: %4\n"
                              "Vendor Identifier: %5\n"
                              "Product Identifier: %6\n"));

        s = s.arg(info.portName()).arg(info.systemLocation())
                .arg(info.description()).arg(info.manufacturer())
                .arg(info.vendorIdentifier()).arg(info.productIdentifier());

        qDebug() << s;
        ui->serialPortComboBox->insertItem(i, info.portName());
    }
}

void LoggerDialog::onConnectButtonPressed()
{
    // TODO hardcoded settings
    _serialPort->setRate(SerialPort::Rate115200);
    _serialPort->setDataBits(SerialPort::Data8);
    _serialPort->setParity(SerialPort::NoParity);
    _serialPort->setFlowControl(SerialPort::UnknownFlowControl);
    _serialPort->setStopBits(SerialPort::TwoStop);

    if(!_serialPort->open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(this, "Logger", "Serial port connection failed, reason: " + errorString(), QMessageBox::Ok);
    }
}

void LoggerDialog::onSerialPortSelected(int index)
{
    _serialPort->setPort(_serialPorts.at(index));
}

void LoggerDialog::onLogMessageReceived(QString logMessage)
{
    appendToLog(logMessage);
}


void LoggerDialog::onPacketReceived(QString packet)
{
    appendToLog("[packet] " + packet);
}

void LoggerDialog::appendToLog(QString msg)
{
    ui->parsedOutputPlainTextEdit->appendPlainText(msg);
    QScrollBar *scrollbar = ui->parsedOutputPlainTextEdit->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

QString LoggerDialog::errorString()
{
    switch(_serialPort->error())
    {
        case SerialPort::NoError:
            return "no error";
        case SerialPort::NoSuchDeviceError:
            return "no such device";
        case SerialPort::PermissionDeniedError:
            return "permission denied";
        case SerialPort::DeviceAlreadyOpenedError:
            return "device is already opened";
        case SerialPort::DeviceIsNotOpenedError:
            return "device is not opened";
        case SerialPort::FramingError:
            return "framing error";
        case SerialPort::ParityError:
            return "parity error";
        case SerialPort::BreakConditionError:
            return "break condition error";
        case SerialPort::IoError:
            return "i/o error";
        case SerialPort::UnsupportedPortOperationError:
            return "unsupported port operation error";
        case SerialPort::UnknownPortError:
            return "unknown port error";
        default:
            return "unknown error";
    }
}

