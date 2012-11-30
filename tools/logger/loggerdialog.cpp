#include "loggerdialog.h"
#include "ui_loggerdialog.h"

#include <QDebug>

LoggerDialog::LoggerDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoggerDialog)
{
    ui->setupUi(this);

    _serialPort = new SerialPort(this);
    _receivedDataQueue = new QQueue<unsigned char>();

    connect(ui->serialPortComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onSerialPortSelected(int)));
    connect(ui->connectButton, SIGNAL(pressed()), SLOT(onConnectButtonPressed()));
    connect(_serialPort, SIGNAL(readyRead()), SLOT(onDataAvailable()));

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
    if(!_serialPort->open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(this, "Logger", "Serial port connection failed, reason: " + errorString(), QMessageBox::Ok);
    }

    // TODO hardcoded settings
    _serialPort->setRate(SerialPort::Rate115200);
    _serialPort->setDataBits(SerialPort::Data8);
    _serialPort->setParity(SerialPort::NoParity);
    _serialPort->setFlowControl(SerialPort::UnknownFlowControl);
    _serialPort->setStopBits(SerialPort::TwoStop);
}

void LoggerDialog::onSerialPortSelected(int index)
{
    _serialPort->setPort(_serialPorts.at(index));
}

void LoggerDialog::onDataAvailable()
{
    QByteArray data = _serialPort->readAll();
    for(int i = 0; i < data.size(); i++)
    {
        ui->outputPlainTextEdit->insertPlainText(QString().sprintf("0x%02x ", (unsigned char)data.constData()[i]));
        _receivedDataQueue->enqueue((unsigned char)data.constData()[i]);
    }    

    parseReceivedData();
}

void LoggerDialog::parseReceivedData()
{
    if(_receivedDataQueue->size() < 3)
        return;

    unsigned char start = _receivedDataQueue->dequeue();
    while(start != 0xDD && !_receivedDataQueue->isEmpty())
    {
        qDebug() << "skipping unexpected data" << QString().sprintf("0x%02x", start);
        start = _receivedDataQueue->dequeue();
    }

    if(_receivedDataQueue->size() < 2 || _receivedDataQueue->size() < _receivedDataQueue->at(1) + 2)
    {
        //  not a full packet, reinsert header and wait for more data ...
        _receivedDataQueue->insert(0, 0xDD);
        return;
    }

    uint type = _receivedDataQueue->dequeue();
    uint len = _receivedDataQueue->dequeue();
    if(type == 0x00)
    {
        QString msg = "[packet data] ";
        for(int i = 0; i < len; i++)
        {
            msg += QString().sprintf("0x%02x ",_receivedDataQueue->dequeue());
        }

        appendToLog(msg, ui->parsedOutputPlainTextEdit);
    }
    if(type == 0x01)
    {
        QString msg;
        for(int i = 0; i < len; i++)
        {
            msg += QString(_receivedDataQueue->dequeue());
        }

        appendToLog(msg, ui->parsedOutputPlainTextEdit);
    }
}

void LoggerDialog::appendToLog(QString msg, QPlainTextEdit *textEdit)
{
    textEdit->appendPlainText(msg + "\n");
    QScrollBar *scrollbar = textEdit->verticalScrollBar();
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

