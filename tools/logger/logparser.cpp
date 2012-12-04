#include "logparser.h"

#include <QDebug>

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
        //ui->outputPlainTextEdit->insertPlainText(QString().sprintf("0x%02x ", (unsigned char)data.constData()[i]));
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

    uint type = _receivedDataQueue->dequeue();
    uint len = _receivedDataQueue->dequeue();
    if(type == 0x00)
    {
        QString msg;
        for(int i = 0; i < len; i++)
        {
            msg += QString().sprintf("0x%02x ",_receivedDataQueue->dequeue());
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

    parseReceivedData();
}
