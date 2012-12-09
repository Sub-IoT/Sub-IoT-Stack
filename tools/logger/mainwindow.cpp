#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore/QThread>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _serialPortComboBox = new QComboBox(this);
    connect(_serialPortComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onSerialPortSelected(int)));

    initToolbar();

    _serialPort = new SerialPort(this);
    _logParser = new LogParser(_serialPort);

    connect(_logParser, SIGNAL(logMessageReceived(QString)), SLOT(onLogMessageReceived(QString)));

    QThread readerThread;
    _logParser->moveToThread(&readerThread);
    readerThread.start();

    detectSerialPorts();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initToolbar()
{
    ui->toolBar->addWidget(_serialPortComboBox);
    ui->toolBar->addAction(ui->connectAction);
}

void MainWindow::detectSerialPorts()
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
        _serialPortComboBox->insertItem(i, info.portName());
    }
}

void MainWindow::on_connectAction_triggered(bool connect)
{
    // TODO disconnect

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

void MainWindow::onSerialPortSelected(int index)
{
    _serialPort->setPort(_serialPorts.at(index));
}

void MainWindow::onLogMessageReceived(QString logMessage)
{
    appendToLog(logMessage);
}

void MainWindow::appendToLog(QString msg)
{
    ui->parsedOutputPlainTextEdit->appendPlainText(msg);
    QScrollBar *scrollbar = ui->parsedOutputPlainTextEdit->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

QString MainWindow::errorString()
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
