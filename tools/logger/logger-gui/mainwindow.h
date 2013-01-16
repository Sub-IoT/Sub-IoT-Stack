#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

#include <qcustomplot/qcustomplot.h>

#include "logparser.h"


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

private slots:
    void onLogMessageReceived(QString logMessage);
    void onPacketParsed(Packet packet);
    void on_connectAction_triggered(bool checked);
    void on_restartAction_triggered();

private:
    void initToolbar();
    void initStatusbar();
    QString serialErrorString() const;
    void parseReceivedData();
    void appendToLog(QString msg);
    void updateStatus();

    Ui::MainWindow *ui;
    QComboBox* _serialPortComboBox;
    QLabel* _packetsReceivedCountLabel;
    QLabel* _crcErrorsCountLabel;
    QLabel* _connectionStatusLabel;

    QIODevice* _ioDevice;
    LogParser* _logParser;
    QThread* _parserThread;

    int _packetsReceivedCount;
    int _crcErrorCount;
    int _bytesSkippedCount;
    QVector<double> _rssValues;
    QVector<double> _timestampValues;
};

#endif // MAINWINDOW_H
