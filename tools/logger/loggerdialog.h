#ifndef LOGGERDIALOG_H
#define LOGGERDIALOG_H

#include <QtGui>
#include <QtCore/QQueue>

#include <serialport/serialport.h>
#include <serialport/serialportinfo.h>

QT_USE_NAMESPACE_SERIALPORT

namespace Ui {
    class LoggerDialog;
}

class LoggerDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoggerDialog(QWidget *parent = 0);
    ~LoggerDialog();
    
protected slots:
    void onSerialPortSelected(int index);
    void onDataAvailable();
    void onConnectButtonPressed();

private:
    void detectSerialPorts();
    QString errorString();
    void parseReceivedData();
    void appendToLog(QString msg, QPlainTextEdit* textEdit);

    Ui::LoggerDialog *ui;

    QList<SerialPortInfo> _serialPorts;
    SerialPort* _serialPort;
    QQueue<unsigned char>* _receivedDataQueue;
};

#endif // LOGGERDIALOG_H
