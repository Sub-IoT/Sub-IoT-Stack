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
#include "serialport_symbian_p.h"

#include <e32base.h>
//#include <e32test.h>
#include <c32comm.h>
#include <f32file.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE_SERIALPORT

// Physical device driver.
#ifdef __WINS__
_LIT(KPddName, "ECDRV");
#else // defined (__EPOC32__)
_LIT(KPddName, "EUART");
#endif

// Logical native device driver.
_LIT(KLddName,"ECOMM");

// Modules names.
_LIT(KRS232ModuleName , "ECUART");
_LIT(KBluetoothModuleName , "BTCOMM");
_LIT(KInfraRedModuleName , "IRCOMM");
_LIT(KACMModuleName, "ECACM");

// Return false on error load.
static bool loadDevices()
{
    TInt r = KErrNone;
#ifdef __WINS__
    RFs fileServer;
    r = User::LeaveIfError(fileServer.Connect());
    if (r != KErrNone)
        return false;
    fileServer.Close ();
#endif

    r = User::LoadPhysicalDevice(KPddName);
    if (r != KErrNone && r != KErrAlreadyExists)
        return false; //User::Leave(r);

    r = User::LoadLogicalDevice(KLddName);
    if (r != KErrNone && r != KErrAlreadyExists)
        return false; //User::Leave(r);

#ifndef __WINS__
    r = StartC32();
    if (r != KErrNone && r != KErrAlreadyExists)
        return false; //User::Leave(r);
#endif

    return true;
}

QList<SerialPortInfo> SerialPortInfo::availablePorts()
{
    QList<SerialPortInfo> ports;

    if (!loadDevices())
        return ports;

    RCommServ server;
    TInt r = server.Connect();
    if (r != KErrNone)
        return ports; //User::LeaveIfError(r);

    TSerialInfo nativeInfo; // Native Symbian OS port info class.
    QString s("%1::%2");

    // FIXME: Get info about RS232 ports.
    r = server.LoadCommModule(KRS232ModuleName);
    //User::LeaveIfError(r);
    if (r == KErrNone) {
        r = server.GetPortInfo(KRS232ModuleName, nativeInfo);
        if (r == KErrNone) {
            //
            for (quint32 i = nativeInfo.iLowUnit; i < nativeInfo.iHighUnit + 1; ++i) {

                SerialPortInfo info; // My (desired) info class.

                info.d_ptr->device = s
                        .arg(QString::fromUtf16(nativeInfo.iName.Ptr(), nativeInfo.iName.Length()))
                        .arg(i);
                info.d_ptr->portName = info.d_ptr->device;
                info.d_ptr->description =
                        QString::fromUtf16(nativeInfo.iDescription.Ptr(), nativeInfo.iDescription.Length());
                info.d_ptr->manufacturer = QString(QObject::tr("Unknown."));
                ports.append(info);
            }
        }
    }

    // FIXME: Get info about Bluetooth ports.
    r = server.LoadCommModule(KBluetoothModuleName);
    //User::LeaveIfError(r);
    if (r == KErrNone) {
        r = server.GetPortInfo(KBluetoothModuleName, nativeInfo);
        if (r == KErrNone) {
            //
            for (quint32 i = nativeInfo.iLowUnit; i < nativeInfo.iHighUnit + 1; ++i) {

                SerialPortInfo info; // My (desired) info class.

                info.d_ptr->device = s
                        .arg(QString::fromUtf16(nativeInfo.iName.Ptr(), nativeInfo.iName.Length()))
                        .arg(i);
                info.d_ptr->portName = info.d_ptr->device;
                info.d_ptr->description =
                        QString::fromUtf16(nativeInfo.iDescription.Ptr(), nativeInfo.iDescription.Length());
                info.d_ptr->manufacturer = QString(QObject::tr("Unknown."));
                ports.append(info);
            }
        }
    }

    // FIXME: Get info about InfraRed ports.
    r = server.LoadCommModule(KInfraRedModuleName);
    //User::LeaveIfError(r);
    if (r == KErrNone) {
        r = server.GetPortInfo(KInfraRedModuleName, nativeInfo);
        if (r == KErrNone) {
            //
            for (quint32 i = nativeInfo.iLowUnit; i < nativeInfo.iHighUnit + 1; ++i) {

                SerialPortInfo info; // My (desired) info class.

                info.d_ptr->device = s
                        .arg(QString::fromUtf16(nativeInfo.iName.Ptr(), nativeInfo.iName.Length()))
                        .arg(i);
                info.d_ptr->portName = info.d_ptr->device;
                info.d_ptr->description =
                        QString::fromUtf16(nativeInfo.iDescription.Ptr(), nativeInfo.iDescription.Length());
                info.d_ptr->manufacturer = QString(QObject::tr("Unknown."));
                ports.append(info);
            }
        }
    }

    // FIXME: Get info about ACM ports.
    r = server.LoadCommModule(KACMModuleName);
    //User::LeaveIfError(r);
    if (r == KErrNone) {
        r = server.GetPortInfo(KACMModuleName, nativeInfo);
        if (r == KErrNone) {
            //
            for (quint32 i = nativeInfo.iLowUnit; i < nativeInfo.iHighUnit + 1; ++i) {

                SerialPortInfo info; // My (desired) info class.

                info.d_ptr->device = s
                        .arg(QString::fromUtf16(nativeInfo.iName.Ptr(), nativeInfo.iName.Length()))
                        .arg(i);
                info.d_ptr->portName = SerialPortPrivate::portNameFromSystemLocation(info.d_ptr->device);
                info.d_ptr->description =
                        QString::fromUtf16(nativeInfo.iDescription.Ptr(), nativeInfo.iDescription.Length());
                info.d_ptr->manufacturer = QString(QObject::tr("Unknown."));
                ports.append(info);
            }
        }
    }

    return ports;
}

QList<qint32> SerialPortInfo::standardRates()
{
    return SerialPortPrivate::standardRates();
}

bool SerialPortInfo::isBusy() const
{
    if (!loadDevices())
        return false;

    RCommServ server;
    TInt r = server.Connect();
    if (r != KErrNone)
        return false;

    RComm port;
    TPtrC portName(static_cast<const TUint16*>(systemLocation().utf16()), systemLocation().length());
    r = port.Open(server, portName, ECommExclusive);
    if (r == KErrNone)
        port.Close();
    return r == KErrLocked;
}

bool SerialPortInfo::isValid() const
{
    if (!loadDevices())
        return false;

    RCommServ server;
    TInt r = server.Connect();
    if (r != KErrNone)
        return false;

    RComm port;
    TPtrC portName(static_cast<const TUint16*>(systemLocation().utf16()), systemLocation().length());
    r = port.Open(server, portName, ECommExclusive);
    if (r == KErrNone)
        port.Close();
    return r == KErrNone || r == KErrLocked;
}

QT_END_NAMESPACE_SERIALPORT
