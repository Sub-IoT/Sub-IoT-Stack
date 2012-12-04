#ifndef LOGPARSER_H
#define LOGPARSER_H

#include <QObject>
#include <QQueue>

#include <serialport/serialport.h>

QT_USE_NAMESPACE_SERIALPORT

class LogParser : public QObject
{
    Q_OBJECT
public:
    explicit LogParser(SerialPort* serialPort, QObject *parent = 0);

signals:
    void logMessageReceived(QString logMessage);
    void packetReceived(QString packet);

protected slots:
    void onDataAvailable();

private:
    void parseReceivedData();
    void parsePhyRxResult(QByteArray phyRxResult);

    SerialPort* _serialPort;
    QQueue<unsigned char>* _receivedDataQueue;
};

#endif // LOGPARSER_H
