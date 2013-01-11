#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

#include <serialport/serialport.h>
#include <serialport/serialportinfo.h>

#include <qcustomplot/qcustomplot.h>

#include "logparser.h"

QT_USE_NAMESPACE_SERIALPORT

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void updatePlot();
protected slots:
    void onSerialPortSelected(int index);
    void onLogMessageReceived(QString logMessage);
    void onPacketParsed(Packet packet);

    void on_connectAction_triggered(bool connect);
    
private slots:
    void on_restartAction_triggered();

private:
    void initToolbar();
    void initStatusbar();
    void detectSerialPorts();
    QString errorString();
    void parseReceivedData();
    void appendToLog(QString msg);
    void updateStatus();

    Ui::MainWindow *ui;
    QComboBox* _serialPortComboBox;
    QLabel* _packetsReceivedCountLabel;
    QLabel* _crcErrorsCountLabel;
    QLabel* _connectionStatusLabel;

    QList<SerialPortInfo> _serialPorts;
    SerialPort* _serialPort;
    LogParser* _logParser;

    int _packetsReceivedCount;
    int _crcErrorCount;
    int _bytesSkippedCount;
    QVector<double> _rssValues;
    QVector<double> _timestampValues;
};

#endif // MAINWINDOW_H
