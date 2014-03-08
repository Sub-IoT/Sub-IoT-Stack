#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QScrollBar>
#include <QtCore/QThread>
#include <QDebug>

#include <QtSerialPort/QSerialPort>

#include "bytearrayutils.h"

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

    _timestampMin = QDateTime::currentDateTime().toTime_t();
    _timestampMax = QDateTime::currentDateTime().toTime_t();

    ui->plotWidget->xAxis->setLabel("timestamp");
    ui->plotWidget->yAxis->setLabel("RSS [dBm]");
    ui->plotWidget->yAxis->setRange(-120, 0);
    ui->plotWidget->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->plotWidget->xAxis->setDateTimeFormat("hh:mm:ss:zzz");
    ui->plotWidget->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    ui->plotWidget->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
    ui->plotWidget->legend->setVisible(true);
    ui->plotWidget->legend->setFont(QFont(QFont().family(), 8));
    ui->plotWidget->legend->setPositionStyle(QCPLegend::psBottomLeft);
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

        QSerialPort* serial = qobject_cast<QSerialPort*>(_ioDevice);
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
                QSerialPort* serialPort = new QSerialPort(this);
                _ioDevice = serialPort;
                serialPort->setPort( dlg.serialPortInfo());
                if(serialPort->open(QIODevice::ReadWrite))
                {
                    // TODO hardcoded settings
                    if(!serialPort->setBaudRate(QSerialPort::Baud115200) ||
                        !serialPort->setDataBits(QSerialPort::Data8) ||
                        !serialPort->setParity(QSerialPort::NoParity) ||
                        !serialPort->setFlowControl(QSerialPort::NoFlowControl) ||
                        !serialPort->setStopBits(QSerialPort::TwoStop))
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

           _logParser->setParent(0);
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
    ui->plotWidget->clearGraphs();
    _timestampMin = QDateTime::currentDateTime().toTime_t();
    _timestampMax = QDateTime::currentDateTime().toTime_t();
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
    for(int i = 0; i < _rssValues.keys().count(); i++)
    {
        QString sourceId = _rssValues.keys()[i];
        ui->plotWidget->graph(i)->setData(_timestampValues[sourceId], _rssValues[sourceId]);

        if(_timestampValues[sourceId].value(0) < _timestampMin)
            _timestampMin = _timestampValues[sourceId].value(0);

        if(_timestampValues[sourceId].value(_timestampValues[sourceId].count()-1) > _timestampMax)
            _timestampMax = _timestampValues[sourceId].value(_timestampValues[sourceId].count()-1);
    }

    ui->plotWidget->xAxis->setRange(_timestampMin, _timestampMax);
    ui->plotWidget->replot();
}

void MainWindow::onPacketParsed(Packet packet)
{
    _packetsReceivedCount++;

    if(!packet.isCrcValid())
    {
        _crcErrorCount++;
    }

    QString sourceId = ByteArrayUtils::toString(packet.sourceId()).replace(" 0x", "").replace("0x", "");
    if(!_rssValues.contains(sourceId))
    {
        _rssValues.insert(sourceId, QVector<double>());
        _timestampValues.insert(sourceId, QVector<double>());

        QCPGraph* deviceGraph = ui->plotWidget->addGraph();
        deviceGraph->setScatterStyle(QCP::ssDisc);
        deviceGraph->setScatterSize(5);
        deviceGraph->setName(sourceId);
        ensureDistinctColors();
    }

    _rssValues[sourceId].append(packet.rss());
    _timestampValues[sourceId].append(packet.timestamp().toTime_t());
    updatePlot();

    updateStatus();
}

void MainWindow::ensureDistinctColors()
{
    double golden_ratio_conjugate = 0.618033988749895;
    double h = qrand() / 100.0;
    for(int i = 0; i < ui->plotWidget->graphCount(); i++)
    {
        h += golden_ratio_conjugate;
        h = fmod(h, 1.0);
        QColor color = QColor::fromHsvF(h, 0.5, 0.95);
        ui->plotWidget->graph(i)->setPen(QPen(color));
    }
}

QString MainWindow::serialErrorString() const
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
