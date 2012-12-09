#include "packet.h"

#include "phy/phy.h"
#include "log.h"

#include "bytearrayutils.h"
#include "hexdump.h"

Packet::Packet()
{
    _timestamp = QDateTime::currentDateTime();
    _hasDllInformation = false;
}

QByteArray Packet::rawPacket() const
{
    return _rawPacket;
}

void Packet::parsePhyRx(QByteArray packetData)
{
    const char* data = packetData.constData();

    _crcOk = data[0];
    _rssi = data[1];
    _eirp = data[2];
    _lqi = data[3];
    _length = data[4];
    _rawPacket = QByteArray(&data[5], _length);
}

void Packet::parseRawPacket()
{
    Q_ASSERT_X(_hasDllInformation, "parseRawPacket", "Cannot parse when ng DLL RX log received");

    const char* data = _rawPacket.constData();

    if(_frameType == FrameTypeBackgroundFrame)
    {
        // TODO
        qCritical("not implemented yet");
    }
    else if(_frameType == FrameTypeForegroundFrame)
    {
        data += 2;
        _subnet = *data; data++;// TODO define
        u8 frame_ctl = *data; data++; // TODO define

        if(frame_ctl & FRAME_CTL_DLLS)
        {
            // TODO
            qCritical("not implemented yet");
        }

        if(frame_ctl & FRAME_CTL_EN_ADDR)
        {
            _dialogId = *data; data++;
            u8 flags = *data; data++;
            u8 addressingOptions = (flags & 0xC0) >> 6;
            bool vidEnabled = (flags & 0x20);
            bool nlsEnabled = (flags & 0x10);
            // TODO frame continuity
            // TODO CRC32
            // TODO NM2
            // TODO frame type

            if(vidEnabled)
            {
                _sourceId = QByteArray(data, 2);
                data += 2;
            }
            else
            {
                _sourceId = QByteArray(data, 8);
                data += 8;
            }

            if(nlsEnabled && addressingOptions == 0)
            {
                // TODO parse target ID header
                qCritical("not implemented yet");
            }

            data++; // TODO what is this?
            data++; // isfid
            data++; // isfoffset
            u8 payloadLength = *data; data++;
            _payload = QByteArray(data, payloadLength);
        }
    }

}

void Packet::parseDllRx(QByteArray data)
{
    if(data.size() != 1) // TODO define
        qCritical("Packet::parseDllRx unexpected data size");

    _frameType = (Frame_Type)data.constData()[0];
    _hasDllInformation = true;

    parseRawPacket();
}

bool Packet::hasDllInformation() const
{
    return _hasDllInformation;
}

QDateTime Packet::timestamp() const
{
    return _timestamp;
}

QString Packet::toString() const
{
    char buffer[1000]; // TODO max size?
    hexdump(buffer, _rawPacket.constData(), _rawPacket.length(), NULL, 0);
    QString hex = QString("Packet:\n%1").arg(buffer);
    QString packetDescription =
            QString("PHY RX:\n" \
                 "\tLength: %1 bytes\n" \
                 "\tCRC:   %2\n" \
                 "\tRSSI:   %3 dBm\n" \
                 "\tEIRP:   %4 dBm\n" \
                 "\tLQI:   %5\n")
            .arg(_length)
            .arg(_crcOk ? "OK" : "NOT OK")
            .arg(_rssi)
            .arg(_eirp)
            .arg(_lqi);

    packetDescription.append(QString("DLL:\n" \
               "\tFrame type: %1\n" \
               "\tSubnet: %2\n" \
               "\tDialog ID: %3\n" \
               "\tSource ID: %4\n" \
               "\tPayload: %5\n")
            .arg(_frameType == FrameTypeForegroundFrame? "foreground" : "background")
            .arg(_subnet)
            .arg(_dialogId)
            .arg(ByteArrayUtils::toString(_sourceId))
            .arg(ByteArrayUtils::toString(_payload))
            );

    return hex.append(packetDescription);
}
