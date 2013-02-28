#ifndef LOGPARSER_H
#define LOGPARSER_H

#include <QObject>
#include <QQueue>

#include <serialport/serialport.h>

#include "packet.h"

QT_USE_NAMESPACE_SERIALPORT

class LogParser : public QObject
{
    Q_OBJECT
public:
    explicit LogParser(SerialPort* serialPort, QObject *parent = 0);

signals:
    void packetParsed(Packet packet);
    void logMessageReceived(QString logMessage);

protected slots:
    void onDataAvailable();

private:
    void parseReceivedData();
    void parsePhyRxResult(QByteArray frameData);
    void parseDllRxResult(QByteArray frameData);

    SerialPort* _serialPort;
    QQueue<unsigned char>* _receivedDataQueue;
    QList<Packet> _packets;
};

#endif // LOGPARSER_H
