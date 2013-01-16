#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore/QThread>
#include <QDebug>

#include <serialport.h>
QT_USE_NAMESPACE_SERIALPORT

#include "connectdialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{    
    qRegisterMetaType<Packet>();

    ui->setupUi(this);

    _packetsReceivedCount = 0;
    _crcErrorCount = 0;
    _bytesSkippedCount = 0;

    _ioDevice = NULL;
    _logParser = NULL;
    _parserThread = new QThread();

    initToolbar();
    initStatusbar();

    ui->plotWidget->addGraph();
    ui->plotWidget->graph(0)->setScatterStyle(QCP::ssDisc);
    ui->plotWidget->graph(0)->setScatterSize(5);
    ui->plotWidget->xAxis->setLabel("timestamp");
    ui->plotWidget->yAxis->setLabel("RSS [dBm]");
    ui->plotWidget->yAxis->setRange(-120, 0);
    ui->plotWidget->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->plotWidget->xAxis->setDateTimeFormat("hh:mm:ss:zzz");
    ui->plotWidget->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    ui->plotWidget->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initToolbar()
{
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
    if(_ioDevice && _ioDevice->isOpen())
    {
        QString device;

        SerialPort* serial = qobject_cast<SerialPort*>(_ioDevice);
        if(serial != NULL)
            device = serial->portName();

        QFile* file = qobject_cast<QFile*>(_ioDevice);
        if(file != NULL)
            device = file->fileName();

        _connectionStatusLabel->setText(QString("Connected to: %1").arg(device));
    }
    else
    {
       _connectionStatusLabel->setText(QString("Not connected"));
    }

    _packetsReceivedCountLabel->setText(QString("Packets received: %1").arg(_packetsReceivedCount));
    _crcErrorsCountLabel->setText(QString("Packets with CRC errors: %2").arg(_crcErrorCount));
}

void MainWindow::on_connectAction_triggered(bool connect)
{
    if(connect)
    {
        ConnectDialog dlg(this);
        if(dlg.exec() == QDialog::Accepted)
        {
            if(dlg.connectionType() == Serial)
            {
                SerialPort* serialPort = new SerialPort(this);
                _ioDevice = serialPort;
                serialPort->setPort(dlg.serialPortName());
                if(serialPort->open(QIODevice::ReadWrite))
                {
                    // TODO hardcoded settings
                    if(!serialPort->setRate(SerialPort::Rate115200) ||
                        !serialPort->setDataBits(SerialPort::Data8) ||
                        !serialPort->setParity(SerialPort::NoParity) ||
                        !serialPort->setFlowControl(SerialPort::NoFlowControl) ||
                        !serialPort->setStopBits(SerialPort::TwoStop))
                    {
                        serialPort->close();
                        QMessageBox::critical(this, "Logger", "Can't configure serial port, reason: " + serialErrorString());
                        return;
                    }

                }
                else
                {
                    QMessageBox::critical(this, "Logger", "Serial port connection failed, reason: " + serialErrorString(), QMessageBox::Ok);
                    return;
                }
            }
            else if(dlg.connectionType() == File)
            {
                QFile* file = new QFile(dlg.fileName(), this);
                _ioDevice = file;
            }

            _logParser = new LogParser(_ioDevice, this);
            QObject::connect(_logParser, SIGNAL(logMessageReceived(QString)), SLOT(onLogMessageReceived(QString)));
            QObject::connect(_logParser, SIGNAL(packetParsed(Packet)), SLOT(onPacketParsed(Packet)));

            _logParser->moveToThread(_parserThread);
            _parserThread->start();
            _logParser->openDevice();
        }
    }
    else
    {
        _ioDevice->close();
    }

    updateStatus();
}


void MainWindow::on_restartAction_triggered()
{
    ui->parsedOutputPlainTextEdit->clear();
    _packetsReceivedCount = 0;
    _crcErrorCount = 0;
    updateStatus();

    _timestampValues.clear();
    _rssValues.clear();
    updatePlot();
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

void MainWindow::updatePlot()
{
    ui->plotWidget->graph(0)->setData(_timestampValues, _rssValues);
    if(_timestampValues.size() > 0)
        ui->plotWidget->xAxis->setRange(_timestampValues.first(), _timestampValues.last());
    ui->plotWidget->replot();
}

void MainWindow::onPacketParsed(Packet packet)
{
    _packetsReceivedCount++;

    if(!packet.isCrcValid())
    {
        _crcErrorCount++;
    }

    _rssValues.append(packet.rss()); // TODO separate graph per device id
    _timestampValues.append(packet.timestamp().toTime_t());
    updatePlot();

    updateStatus();
}

QString MainWindow::serialErrorString() const
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
