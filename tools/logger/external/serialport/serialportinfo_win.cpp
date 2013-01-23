/****************************************************************************
**
** Copyright (C) 2011-2012 Denis Shienkov <scapig2@yandex.ru>
** Copyright (C) 2011 Sergey Belyashov <Sergey.Belyashov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt-project.org/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "serialportinfo.h"
#include "serialportinfo_p.h"
#include "serialport_win_p.h"

#ifndef Q_OS_WINCE
#include <initguid.h>
#include <setupapi.h>
#endif

#include <QtCore/qvariant.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE_SERIALPORT

#ifndef Q_OS_WINCE

static const GUID guidsArray[] =
{
    // Windows Ports Class GUID
    { 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },
    // Virtual Ports Class GUID (i.e. com0com and etc)
    { 0xDF799E12, 0x3C56, 0x421B, { 0xB2, 0x98, 0xB6, 0xD3, 0x64, 0x2B, 0xC8, 0x78 } },
    // Windows Modems Class GUID
    { 0x4D36E96D, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },
    // Eltima Virtual Serial Port Driver v4 GUID
    { 0xCC0EF009, 0xB820, 0x42F4, { 0x95, 0xA9, 0x9B, 0xFA, 0x6A, 0x5A, 0xB7, 0xAB } },
    // Advanced Virtual COM Port GUID
    { 0x9341CD95, 0x4371, 0x4A37, { 0xA5, 0xAF, 0xFD, 0xB0, 0xA9, 0xD1, 0x96, 0x31 } },
};

static const wchar_t portKeyName[] = L"PortName";

static QVariant deviceRegistryProperty(HDEVINFO deviceInfoSet,
                                          PSP_DEVINFO_DATA deviceInfoData,
                                          DWORD property)
{
    DWORD dataType = 0;
    DWORD dataSize = 0;
    ::SetupDiGetDeviceRegistryProperty(deviceInfoSet, deviceInfoData,
                                       property, &dataType, NULL, 0, &dataSize);
    QByteArray data(dataSize, 0);
    if (!::SetupDiGetDeviceRegistryProperty(deviceInfoSet, deviceInfoData, property, NULL,
                                            reinterpret_cast<unsigned char*>(data.data()),
                                            dataSize, NULL)
            || !dataSize) {
        return QVariant();
    }

    switch (dataType) {

    case REG_EXPAND_SZ:
    case REG_SZ: {
        return QVariant(QString::fromWCharArray(reinterpret_cast<const wchar_t *>(data.constData())));
    }

    case REG_MULTI_SZ: {
        QStringList list;
        int i = 0;
        forever {
            QString s = QString::fromWCharArray(reinterpret_cast<const wchar_t *>(data.constData()) + i);
            i += s.length() + 1;
            if (s.isEmpty())
                break;
            list.append(s);
        }
        return QVariant(list);
    }

    default:
        break;
    }

    return QVariant();
}

static QString devicePortName(HDEVINFO deviceInfoSet, PSP_DEVINFO_DATA deviceInfoData)
{
    const HKEY key = ::SetupDiOpenDevRegKey(deviceInfoSet, deviceInfoData, DICS_FLAG_GLOBAL,
                                            0, DIREG_DEV, KEY_READ);
    if (key == INVALID_HANDLE_VALUE)
        return QString();

    DWORD dataSize;
    if (::RegQueryValueEx(key, portKeyName, NULL, NULL, NULL, &dataSize) != ERROR_SUCCESS) {
        ::RegCloseKey(key);
        return QString();
    }

    QByteArray data(dataSize, 0);

    if (::RegQueryValueEx(key, portKeyName, NULL, NULL,
                reinterpret_cast<unsigned char *>(data.data()), &dataSize) != ERROR_SUCCESS) {
        ::RegCloseKey(key);
        return QString();
    }
    ::RegCloseKey(key);
    return QString::fromWCharArray(((const wchar_t *)data.constData()));
}

QList<SerialPortInfo> SerialPortInfo::availablePorts()
{
    QList<SerialPortInfo> ports;
    static const int guidCount = sizeof(guidsArray)/sizeof(guidsArray[0]);

    for (int i = 0; i < guidCount; ++i) {
        const HDEVINFO deviceInfoSet = ::SetupDiGetClassDevs(&guidsArray[i], NULL, 0, DIGCF_PRESENT);
        if (deviceInfoSet == INVALID_HANDLE_VALUE)
            return ports;

        SP_DEVINFO_DATA deviceInfoData;
        ::memset(&deviceInfoData, 0, sizeof(deviceInfoData));
        deviceInfoData.cbSize = sizeof(deviceInfoData);

        DWORD index = 0;
        while (::SetupDiEnumDeviceInfo(deviceInfoSet, index++, &deviceInfoData)) {
            SerialPortInfo info;

            QString s = devicePortName(deviceInfoSet, &deviceInfoData);
            if (s.isEmpty() || s.contains(QLatin1String("LPT")))
                continue;

            info.d_ptr->portName = s;
            info.d_ptr->device = SerialPortPrivate::portNameToSystemLocation(s);
            info.d_ptr->description =
                    deviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC).toString();
            info.d_ptr->manufacturer =
                    deviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_MFG).toString();

            s = deviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID).toStringList().first().toUpper();
            info.d_ptr->vendorIdentifier = s.mid(s.indexOf(QLatin1String("VID_")) + 4, 4);
            info.d_ptr->productIdentifier = s.mid(s.indexOf(QLatin1String("PID_")) + 4, 4);

            ports.append(info);
        }
        ::SetupDiDestroyDeviceInfoList(deviceInfoSet);
    }
    return ports;
}

#endif

// common part

QList<qint32> SerialPortInfo::standardRates()
{
    return SerialPortPrivate::standardRates();
}

bool SerialPortInfo::isBusy() const
{
    const HANDLE descriptor = ::CreateFile(reinterpret_cast<const wchar_t*>(systemLocation().utf16()),
                                           GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (descriptor == INVALID_HANDLE_VALUE) {
        if (::GetLastError() == ERROR_ACCESS_DENIED)
            return true;
    } else {
        ::CloseHandle(descriptor);
    }
    return false;
}

bool SerialPortInfo::isValid() const
{
    const HANDLE descriptor = ::CreateFile(reinterpret_cast<const wchar_t*>(systemLocation().utf16()),
                                           GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (descriptor == INVALID_HANDLE_VALUE) {
        if (::GetLastError() != ERROR_ACCESS_DENIED)
            return false;
    } else {
        ::CloseHandle(descriptor);
    }
    return true;
}

QT_END_NAMESPACE_SERIALPORT
