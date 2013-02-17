#ifndef LOGPARSER_H
#define LOGPARSER_H

#include <QObject>
#include <QQueue>

#include "packet.h"

class LogParser : public QObject
{
    Q_OBJECT
public:
    explicit LogParser(QIODevice* ioDevice, QObject *parent = 0);
    void openDevice();

signals:
    void packetParsed(Packet packet);
    void logMessageReceived(QString logMessage);

protected slots:
    void onDataAvailable();

private:
    void parseReceivedData();
    void parsePhyRxResult(QByteArray frameData);
    void parseDllRxResult(QByteArray frameData);

    QIODevice* _ioDevice;
    QQueue<unsigned char>* _receivedDataQueue;
    QList<Packet> _packets;
};

#endif // LOGPARSER_H
