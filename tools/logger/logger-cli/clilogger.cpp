#include "clilogger.h"

#include <serialport.h>
#include <serialportinfo.h>

QT_USE_NAMESPACE_SERIALPORT

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
        SerialPort* serial = new SerialPort(arg, this);
        _ioDevice = serial;
        if(serial->open(QIODevice::ReadWrite))
        {
            // TODO hardcoded settings
            if(!serial->setRate(SerialPort::Rate115200) ||
                !serial->setDataBits(SerialPort::Data8) ||
                !serial->setParity(SerialPort::NoParity) ||
                !serial->setFlowControl(SerialPort::NoFlowControl) ||
                !serial->setStopBits(SerialPort::TwoStop))
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
    qDebug() << "msg: " << msg;
}

bool CliLogger::isSerialPort(QString arg) const
{
    QList<SerialPortInfo> serialPorts = SerialPortInfo::availablePorts();
    for (int i = 0; i < serialPorts.count(); i++)
    {
        const SerialPortInfo &info = serialPorts.at(i);
        if(arg == info.portName())
            return true;
    }

    return false;
}

QString CliLogger::serialErrorString() const
{
    SerialPort* serial = qobject_cast<SerialPort*>(_ioDevice);
    switch(serial->error())
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
