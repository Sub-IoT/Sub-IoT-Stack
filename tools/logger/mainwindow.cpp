#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore/QThread>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _serialPortComboBox = new QComboBox(this);
    connect(_serialPortComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onSerialPortSelected(int)));

    _serialPort = new SerialPort(this);
    _logParser = new LogParser(_serialPort);
    connect(_logParser, SIGNAL(logMessageReceived(QString)), SLOT(onLogMessageReceived(QString)));
    connect(_logParser, SIGNAL(packetParsed(bool)), SLOT(onPacketParsed(bool)));

    initToolbar();
    initStatusbar();

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
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->restartAction);
}

void MainWindow::initStatusbar()
{
    _connectionStatusLabel = new QLabel(ui->statusbar);
    _connectionStatusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    ui->statusbar->addPermanentWidget(_connectionStatusLabel);

    _packetsReceivedCountLabel = new QLabel(ui->statusbar);
    _packetsReceivedCountLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    ui->statusbar->addPermanentWidget(_packetsReceivedCountLabel);

    _crcErrorsCountLabel = new QLabel(ui->statusbar);
    _crcErrorsCountLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    ui->statusbar->addPermanentWidget(_crcErrorsCountLabel);

    updateStatus();
}

void MainWindow::updateStatus()
{
    if(_serialPort->isOpen())
        _connectionStatusLabel->setText(QString("Connected to: %1").arg(_serialPort->portName()));
    else
        _connectionStatusLabel->setText(QString("Not connected"));

    _packetsReceivedCountLabel->setText(QString("Packets received: %1").arg(_packetsReceivedCount));
    _crcErrorsCountLabel->setText(QString("Packets with CRC errors: %2").arg(_crcErrorCount));
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
    if(connect)
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
    else
    {
        _serialPort->close();
    }

    updateStatus();
}


void MainWindow::on_restartAction_triggered()
{
    ui->parsedOutputPlainTextEdit->clear();
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

void MainWindow::onPacketParsed(bool crcOk)
{
    _packetsReceivedCount++;

    if(!crcOk)
    {
        _crcErrorCount++;
    }

    updateStatus();
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
