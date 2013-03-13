#include "bytearrayutils.h"

QString ByteArrayUtils::toString(const QByteArray byteArray)
{
    QString hexString = "";
    for(int i = 0; i < byteArray.size(); i++)
    {
        hexString.append(QString().sprintf("0x%02x ", (unsigned char)byteArray.at(i)));
    }

    return hexString;
}
