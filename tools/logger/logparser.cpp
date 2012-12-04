#include "logparser.h"

#include <QDebug>

#include "phy/phy.h"

LogParser::LogParser(SerialPort* serialPort, QObject *parent) : QObject(parent)
{
    _serialPort = serialPort;
    _receivedDataQueue = new QQueue<unsigned char>();

    connect(_serialPort, SIGNAL(readyRead()), SLOT(onDataAvailable()));
}

void LogParser::onDataAvailable()
{
    QByteArray data = _serialPort->readAll();
    for(int i = 0; i < data.size(); i++)
    {
        _receivedDataQueue->enqueue((unsigned char)data.constData()[i]);
    }

    parseReceivedData();
}

void LogParser::parseReceivedData()
{
    if(_receivedDataQueue->size() < 3)
        return;

    unsigned char start = _receivedDataQueue->dequeue();
    while(start != 0xDD && !_receivedDataQueue->isEmpty())
    {
        qDebug() << "skipping unexpected data" << QString().sprintf("0x%02x", start);
        start = _receivedDataQueue->dequeue();
    }

    if(_receivedDataQueue->size() < 2 || _receivedDataQueue->size() < _receivedDataQueue->at(1) + 2)
    {
        //  not a full packet, reinsert header and wait for more data ...
        _receivedDataQueue->insert(0, 0xDD);
        return;
    }

    u8 type = _receivedDataQueue->dequeue();
    u8 len = _receivedDataQueue->dequeue();
    if(type == 0x00)
    {
        QString msg;
        QByteArray frameData;
        unsigned char byte;
        for(int i = 0; i < len; i++)
        {
            byte = _receivedDataQueue->dequeue();
            frameData.append(byte);
            msg += QString().sprintf("0x%02x ", byte);
        }

        emit packetReceived(msg);
    }
    if(type == 0x01)
    {
        QString msg;
        for(int i = 0; i < len; i++)
        {
            msg += QString(_receivedDataQueue->dequeue());
        }

        emit logMessageReceived(msg);
    }
    if(type == 0x02)
    {
        QString msg;
        QByteArray frameData;
        unsigned char byte;
        for(int i = 0; i < len; i++)
        {
            byte = _receivedDataQueue->dequeue();
            frameData.append(byte);
            msg += QString().sprintf("0x%02x ", byte);
        }

        emit packetReceived(msg);
        parsePhyRxResult(frameData);
    }

    parseReceivedData();
}

void LogParser::parsePhyRxResult(QByteArray phyRxResult)
{
    phy_rx_res_t* phy_result = (phy_rx_res_t*)phyRxResult.constData();
    QString logMessage = QString("PHY RX:\n" \
                                 "\tLength: %1 bytes\n" \
                                 "\tCRC:   %2\n" \
                                 "\tRSSI:   %3 dBm\n" \
                                 "\tEIRP:   %4 dBm\n" \
                                 "\tLQI:   %5\n")
                            .arg(phy_result->len)
                            .arg(phy_result->crc_ok ? "OK" : "NOT OK")
                            .arg(phy_result->rssi)
                            .arg(phy_result->eirp)
                            .arg(phy_result->lqi);

    emit logMessageReceived(logMessage);
}
