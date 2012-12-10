#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

#include <serialport/serialport.h>
#include <serialport/serialportinfo.h>

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

protected slots:
    void onSerialPortSelected(int index);
    void onLogMessageReceived(QString logMessage);
    void onPacketParsed(bool crcOk);

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

    QList<SerialPortInfo> _serialPorts;
    SerialPort* _serialPort;
    LogParser* _logParser;

    int _packetsReceivedCount;
    int _crcErrorCount;
    int _bytesSkippedCound;
};

#endif // MAINWINDOW_H
