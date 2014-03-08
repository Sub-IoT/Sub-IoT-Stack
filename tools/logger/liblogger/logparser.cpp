#include "logparser.h"

#include <QDebug>

#include "framework/log.h"
#include "phy/phy.h"
#include "dll/dll.h"
#include "hexdump.h"

LogParser::LogParser(QIODevice* ioDevice, QObject *parent) : QObject(parent)
{
    _ioDevice = ioDevice;

    _receivedDataQueue = new QQueue<unsigned char>();

    connect(_ioDevice, SIGNAL(readyRead()), SLOT(onDataAvailable()));
}

void LogParser::openDevice()
{
    _ioDevice->open(QIODevice::ReadOnly);

    if(_ioDevice->bytesAvailable() > 0) // trigger read for implementations like QFile which don't emit readyRead()
        onDataAvailable();
}

void LogParser::onDataAvailable()
{
    QByteArray data = _ioDevice->readAll();
    for(int i = 0; i < data.size(); i++)
    {
        _receivedDataQueue->enqueue((unsigned char)data.constData()[i]);
    }

    parseReceivedData();
}

void LogParser::parseReceivedData()
{

    //qDebug() << "Data size: " << _receivedDataQueue->size();

    if(_receivedDataQueue->size() < 3)
        return;

    unsigned char start = _receivedDataQueue->dequeue();
    while(start != 0xDD && !_receivedDataQueue->isEmpty())
    {
        if(start != 0xFF && start != 0x00) // skip bytes received when cable disconnected
            qDebug() << "skipping unexpected data" << QString().sprintf("0x%02x", start);

        start = _receivedDataQueue->dequeue();
    }

/*
    if(_receivedDataQueue->size() >= 1)
    {
        if(_receivedDataQueue->at(0) != LOG_TYPE_STRING &&
                _receivedDataQueue->at(0) !=LOG_TYPE_PHY_RX_RES &&
                _receivedDataQueue->at(0) != LOG_TYPE_DLL_RX_RES)
        {
            qWarning(qPrintable(QString().sprintf("Unexpected type: 0x%02x, skipping ...", _receivedDataQueue->at(0))));
            return;
        }
    }
*/
    if(_receivedDataQueue->size() < 2 || _receivedDataQueue->size() < _receivedDataQueue->at(1) + 2)
    {
        //  not a full packet, reinsert header and wait for more data ...
        _receivedDataQueue->insert(0, 0xDD);
        return;
    }

    uint8_t type = _receivedDataQueue->dequeue();
    uint8_t len = _receivedDataQueue->dequeue();

    if(type == LOG_TYPE_STRING)
    {
        QString msg;
        for(int i = 0; i < len; i++)
        {
            msg += QString(_receivedDataQueue->dequeue());
        }

        emit logMessageReceived(msg);
    }

    if (type == LOG_TYPE_DATA)
    {
        QString hex= "Data: ";
        unsigned char byte;
        for(int i = 0; i < len; i++)
        {
            byte = _receivedDataQueue->at(i);
            hex.append(QString("%1").arg(byte, 2, 16, QChar('0')).toUpper());
        }
        emit logMessageReceived(hex);
    }

    if(type == LOG_TYPE_PHY_RX_RES)
    {
        QByteArray packetData;
        unsigned char byte;
        for(int i = 0; i < len; i++)
        {
            byte = _receivedDataQueue->dequeue();
            packetData.append(byte);
        }

        Packet p;
        p.parsePhyRx(packetData);
        _packets.append(p);
    }
    if(type == LOG_TYPE_DLL_RX_RES)
    {
        QByteArray packetData;
        unsigned char byte;
        for(int i = 0; i < len; i++)
        {
            byte = _receivedDataQueue->dequeue();
            packetData.append(byte);
        }

        if(_packets.isEmpty())
        {
            qWarning("Received a DllRx while packets empty, probably the first log message received, skipping ...");
            return;
        }

        Packet lastPacket = _packets.last();
        if(lastPacket.hasDllInformation())
        {
            QString msg = "Out of sync, got DLL data while current packet instance already has parsed DLL data";
            qCritical(msg.toLocal8Bit());
            qDebug() << msg;
        }

        lastPacket.parseDllRx(packetData);

        emit logMessageReceived(lastPacket.toString());
        emit packetParsed(lastPacket);
    }

    parseReceivedData();
}


