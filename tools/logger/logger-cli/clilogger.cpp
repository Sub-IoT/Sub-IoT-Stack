#include "clilogger.h"

//#include <serialport.h>
//#include <serialportinfo.h>

#include <QDateTime>

#include <QtSerialPort/QtSerialPort>

#include <logparser.h>

CliLogger::CliLogger(QObject *parent) : QObject(parent)
{
    if(qApp->arguments().size() != 2)
    {
        qDebug() << "Expected 1 parameter (filename or name of serialport (eg 'ttyUSB0')";
        ::exit(-1);
    }

    QString arg = qApp->arguments()[1];
    if(isSerialPort(arg))
    {
        QSerialPort* serial = new QSerialPort(arg, this);
        _ioDevice = serial;
        if(serial->open(QIODevice::ReadWrite))
        {
            // TODO hardcoded settings
            if(!serial->setBaudRate(QSerialPort::Baud115200) ||
                !serial->setDataBits(QSerialPort::Data8) ||
                !serial->setParity(QSerialPort::NoParity) ||
                !serial->setFlowControl(QSerialPort::NoFlowControl) ||
                !serial->setStopBits(QSerialPort::TwoStop))
            {
                serial->close();
                qDebug() << "Can't configure serial port, reason: " + serialErrorString();
                ::exit(-1);
            }
        }
    }
    else if(QFile::exists(arg))
    {
        _ioDevice = new QFile(arg);
    }

    LogParser* parser = new LogParser(_ioDevice, this);
    QObject::connect(parser, SIGNAL(logMessageReceived(QString)), SLOT(onMessageReceived(QString)));
    parser->openDevice();
}

void CliLogger::onMessageReceived(QString msg)
{
    qDebug() << QTime::currentTime().toString("hh:mm:ss.zzz") << " msg: " << msg;
}

bool CliLogger::isSerialPort(QString arg) const
{
    QList<QSerialPortInfo> serialPorts = QSerialPortInfo::availablePorts();
    for (int i = 0; i < serialPorts.count(); i++)
    {
        const QSerialPortInfo &info = serialPorts.at(i);
        if(arg == info.portName())
            return true;
    }

    return false;
}

QString CliLogger::serialErrorString() const
{
    QSerialPort* serial = qobject_cast<QSerialPort*>(_ioDevice);
    switch(serial->error())
    {
        case QSerialPort::NoError:
            return "no error";
        case QSerialPort::DeviceNotFoundError:
            return "device not found";
        case QSerialPort::PermissionError:
            return "permission error";
        case QSerialPort::OpenError:
            return "device is already opened";
        case QSerialPort::FramingError:
            return "framing error";
        case QSerialPort::ParityError:
            return "parity error";
        case QSerialPort::BreakConditionError:
            return "break condition error";
        case QSerialPort::ReadError:
            return "read error";
        case QSerialPort::WriteError:
            return "write error";
        case QSerialPort::UnsupportedOperationError:
            return "unsupported operation error";
        case QSerialPort::UnknownError:
        default:
            return "unknown error";
    }
}
