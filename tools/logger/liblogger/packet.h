#ifndef PACKET_H
#define PACKET_H

#include <QtCore>

#include "dll/dll.h"

class Packet
{
public:
    Packet();

    void parsePhyRx(QByteArray packetData);
    void parseRawPacket();
    void parseDllRx(QByteArray dllRxData);
    bool hasDllInformation() const;

    QByteArray rawPacket() const;
    QDateTime timestamp() const;
    QString toString() const;
    bool isCrcValid() const;
    signed int rss() const;

private:
    QDateTime _timestamp;
    QByteArray _rawPacket;
    uint _length;
    bool _crcOk;
    signed int _eirp;
    signed int _rss;
    uint _lqi;

    bool _hasDllInformation;
    Frame_Type _frameType;
    u8 _subnet;
    u8 _dialogId;
    QByteArray _sourceId;
    QByteArray _payload;
    u8 _spectrumId;
};

Q_DECLARE_METATYPE(Packet)

#endif // PACKET_H
