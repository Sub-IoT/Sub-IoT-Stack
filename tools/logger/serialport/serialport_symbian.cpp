/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <scapig@yandex.ru>
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

#include "serialport_symbian_p.h"

#include <e32base.h>
//#include <e32test.h>
#include <f32file.h>

#include <QtCore/qregexp.h>

QT_BEGIN_NAMESPACE_SERIALPORT

// Physical device driver.
#ifdef __WINS__
_LIT(KPddName, "ECDRV");
#else // defined (__EPOC32__)
_LIT(KPddName, "EUART");
#endif

// Logical  device driver.
_LIT(KLddName,"ECOMM");

// Modules names.
_LIT(KRS232ModuleName, "ECUART");
_LIT(KBluetoothModuleName, "BTCOMM");
_LIT(KInfraRedModuleName, "IRCOMM");
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

SerialPortPrivate::SerialPortPrivate(SerialPort *q)
    : SerialPortPrivateData(q)
    , errnum(KErrNone)
{
}

bool SerialPortPrivate::open(QIODevice::OpenMode mode)
{
    // FIXME: Maybe need added check an ReadWrite open mode?
    Q_UNUSED(mode)

    if (!loadDevices()) {
        portError = SerialPort::UnknownPortError;
        return false;
    }

    RCommServ server;
    errnum = server.Connect();
    if (errnum != KErrNone) {
        portError = decodeSystemError();
        return false;
    }

    if (systemLocation.contains("BTCOMM"))
        errnum = server.LoadCommModule(KBluetoothModuleName);
    else if (systemLocation.contains("IRCOMM"))
        errnum = server.LoadCommModule(KInfraRedModuleName);
    else if (systemLocation.contains("ACM"))
        errnum = server.LoadCommModule(KACMModuleName);
    else
        errnum = server.LoadCommModule(KRS232ModuleName);

    if (errnum != KErrNone) {
        portError = decodeSystemError();
        return false;
    }

    // In Symbian OS port opening only in R/W mode?
    TPtrC portName(static_cast<const TUint16*>(systemLocation.utf16()), systemLocation.length());
    errnum = descriptor.Open(server, portName, ECommExclusive);

    if (errnum != KErrNone) {
        portError = decodeSystemError();
        return false;
    }

    // Save current port settings.
    errnum = descriptor.Config(restoredSettings);
    if (errnum != KErrNone) {
        portError = decodeSystemError();
        return false;
    }

    detectDefaultSettings();
    return true;
}

void SerialPortPrivate::close()
{
    if (restoreSettingsOnClose)
        descriptor.SetConfig(restoredSettings);
    descriptor.Close();
}

SerialPort::Lines SerialPortPrivate::lines() const
{
    SerialPort::Lines ret = 0;

    TUint signalMask = 0;
    descriptor.Signals(signalMask);

    if (signalMask & KSignalCTS)
        ret |= SerialPort::Cts;
    if (signalMask & KSignalDSR)
        ret |= SerialPort::Dsr;
    if (signalMask & KSignalDCD)
        ret |= SerialPort::Dcd;
    if (signalMask & KSignalRNG)
        ret |= SerialPort::Ri;
    if (signalMask & KSignalRTS)
        ret |= SerialPort::Rts;
    if (signalMask & KSignalDTR)
        ret |= SerialPort::Dtr;

    //if (signalMask & KSignalBreak)
    //  ret |=
    return ret;
}

bool SerialPortPrivate::setDtr(bool set)
{
    TInt r;
    if (set)
        r = descriptor.SetSignalsToMark(KSignalDTR);
    else
        r = descriptor.SetSignalsToSpace(KSignalDTR);

    return r == KErrNone;
}

bool SerialPortPrivate::setRts(bool set)
{
    TInt r;
    if (set)
        r = descriptor.SetSignalsToMark(KSignalRTS);
    else
        r = descriptor.SetSignalsToSpace(KSignalRTS);

    return r == KErrNone;
}

bool SerialPortPrivate::flush()
{
    // TODO: Implement me
    return false;
}

bool SerialPortPrivate::clear(SerialPort::Directions dir)
{
    TUint flags = 0;
    if (dir & SerialPort::Input)
        flags |= KCommResetRx;
    if (dir & SerialPort::Output)
        flags |= KCommResetTx;
    TInt r = descriptor.ResetBuffers(flags);
    return r == KErrNone;
}

bool SerialPortPrivate::sendBreak(int duration)
{
    TRequestStatus status;
    descriptor.Break(status, TTimeIntervalMicroSeconds32(duration * 1000));
    return false;
}

bool SerialPortPrivate::setBreak(bool set)
{
    // TODO: Implement me
    return false;
}

qint64 SerialPortPrivate::bytesAvailable() const
{
    return descriptor.QueryReceiveBuffer();
}

qint64 SerialPortPrivate::bytesToWrite() const
{
    // TODO: Implement me
    return 0;
}

qint64 SerialPortPrivate::readFromBuffer(char *data, qint64 maxSize)
{
    // TODO: Implement me
    return -1;
}

qint64 SerialPortPrivate::writeToBuffer(const char *data, qint64 maxSize)
{
    // TODO: Implement me
    return -1;
}

bool SerialPortPrivate::waitForReadyRead(int msec)
{
    // TODO: Implement me
    return false;
}

bool SerialPortPrivate::waitForBytesWritten(int msec)
{
    // TODO: Implement me
    return false;
}

bool SerialPortPrivate::setRate(qint32 rate, SerialPort::Directions dir)
{
    if (dir != SerialPort::AllDirections) {
        portError = SerialPort::UnsupportedPortOperationError;
        return false;
    }

    rate = settingFromRate(rate);
    if (rate)
        currentSettings().iRate = static_cast<TBps>(rate);
    else {
        portError = SerialPort::UnsupportedPortOperationError;
        return false;
    }

    return updateCommConfig();
}

bool SerialPortPrivate::setDataBits(SerialPort::DataBits dataBits)
{
    switch (dataBits) {
    case SerialPort::Data5:
        currentSettings().iDataBits = EData5;
        break;
    case SerialPort::Data6:
        currentSettings().iDataBits = EData6;
        break;
    case SerialPort::Data7:
        currentSettings().iDataBits = EData7;
        break;
    case SerialPort::Data8:
        currentSettings().iDataBits = EData8;
        break;
    default:
        currentSettings().iDataBits = EData8;
        break;
    }

    return updateCommConfig();
}

bool SerialPortPrivate::setParity(SerialPort::Parity parity)
{
    switch (parity) {
    case SerialPort::NoParity:
        currentSettings().iParity = EParityNone;
        break;
    case SerialPort::EvenParity:
        currentSettings().iParity = EParityEven;
        break;
    case SerialPort::OddParity:
        currentSettings().iParity = EParityOdd;
        break;
    case SerialPort::MarkParity:
        currentSettings().iParity = EParityMark;
        break;
    case SerialPort::SpaceParity:
        currentSettings().iParity = EParitySpace;
        break;
    default:
        currentSettings().iParity = EParityNone;
        break;
    }

    return updateCommConfig();
}

bool SerialPortPrivate::setStopBits(SerialPort::StopBits stopBits)
{
    switch (stopBits) {
    case SerialPort::OneStop:
        currentSettings().iStopBits = EStop1;
        break;
    case SerialPort::TwoStop:
        currentSettings().iStopBits = EStop2;
        break;
    default:
        currentSettings().iStopBits = EStop1;
        break;
    }

    return updateCommConfig();
}

bool SerialPortPrivate::setFlowControl(SerialPort::FlowControl flow)
{
    switch (flow) {
    case SerialPort::NoFlowControl:
        currentSettings().iHandshake = KConfigFailDSR;
        break;
    case SerialPort::HardwareControl:
        currentSettings().iHandshake = KConfigObeyCTS | KConfigFreeRTS;
        break;
    case SerialPort::SoftwareControl:
        currentSettings().iHandshake = KConfigObeyXoff | KConfigSendXoff;
        break;
    default:
        currentSettings().iHandshake = KConfigFailDSR;
        break;
    }

    return updateCommConfig();
}

bool SerialPortPrivate::setDataErrorPolicy(SerialPort::DataErrorPolicy policy)
{
    // TODO: Implement me
    return false;
}

bool SerialPortPrivate::notifyRead()
{
    // TODO: Implement me
    return false;
}

bool SerialPortPrivate::notifyWrite()
{
    // TODO: Implement me
    return false;
}

bool SerialPortPrivate::updateCommConfig()
{
    if (descriptor.SetConfig(currentSettings) != KErrNone) {
        portError = SerialPort::UnsupportedPortOperationError;
        return false;
    }
    return true;
}

void SerialPortPrivate::detectDefaultSettings()
{
    // Detect rate.
    inputRate = rateFromSetting(currentSettings().iRate);
    outputRate = inputRate;

    // Detect databits.
    switch (currentSettings().iDataBits) {
    case EData5:
        dataBits = SerialPort::Data5;
        break;
    case EData6:
        dataBits = SerialPort::Data6;
        break;
    case EData7:
        dataBits = SerialPort::Data7;
        break;
    case EData8:
        dataBits = SerialPort::Data8;
        break;
    default:
        dataBits = SerialPort::UnknownDataBits;
        break;
    }

    // Detect parity.
    switch (currentSettings().iParity) {
    case EParityNone:
        parity = SerialPort::NoParity;
        break;
    case EParityEven:
        parity = SerialPort::EvenParity;
        break;
    case EParityOdd:
        parity = SerialPort::OddParity;
        break;
    case EParityMark:
        parity = SerialPort::MarkParity;
        break;
    case EParitySpace:
        parity = SerialPort::SpaceParity;
        break;
    default:
        parity = SerialPort::UnknownParity;
        break;
    }

    // Detect stopbits.
    switch (currentSettings().iStopBits) {
    case EStop1:
        stopBits = SerialPort::OneStop;
        break;
    case EStop2:
        stopBits = SerialPort::TwoStop;
        break;
    default:
        stopBits = SerialPort::UnknownStopBits;
        break;
    }

    // Detect flow control.
    if ((currentSettings().iHandshake & (KConfigObeyXoff | KConfigSendXoff))
            == (KConfigObeyXoff | KConfigSendXoff))
        flow = SerialPort::SoftwareControl;
    else if ((currentSettings().iHandshake & (KConfigObeyCTS | KConfigFreeRTS))
             == (KConfigObeyCTS | KConfigFreeRTS))
        flow = SerialPort::HardwareControl;
    else if (currentSettings().iHandshake & KConfigFailDSR)
        flow = SerialPort::NoFlowControl;
    else
        flow = SerialPort::UnknownFlowControl;
}

SerialPort::PortError SerialPortPrivate::decodeSystemError() const
{
    SerialPort::PortError error;
    switch (errnum) {
    case KErrPermissionDenied:
        error = SerialPort::NoSuchDeviceError;
        break;
    case KErrLocked:
        error = SerialPort::PermissionDeniedError;
        break;
    case KErrAccessDenied:
        error = SerialPort::PermissionDeniedError;
        break;
    default:
        error = SerialPort::UnknownPortError;
        break;
    }
    return error;
}

bool SerialPortPrivate::waitForReadOrWrite(bool *selectForRead, bool *selectForWrite,
                                           bool checkRead, bool checkWrite,
                                           int msecs, bool *timedOut)
{

    // FIXME: I'm not sure in implementation this method.
    // Someone needs to check and correct.

    TRequestStatus timerStatus;
    TRequestStatus readStatus;
    TRequestStatus writeStatus;

    if (msecs > 0)  {
        if (!selectTimer.Handle()) {
            if (selectTimer.CreateLocal() != KErrNone)
                return false;
        }
        selectTimer.HighRes(timerStatus, msecs * 1000);
    }

    if (checkRead)
        descriptor.NotifyDataAvailable(readStatus);

    if (checkWrite)
        descriptor.NotifyOutputEmpty(writeStatus);

    enum { STATUSES_COUNT = 3 };
    TRequestStatus *statuses[STATUSES_COUNT];
    TInt num = 0;
    statuses[num++] = &timerStatus;
    statuses[num++] = &readStatus;
    statuses[num++] = &writeStatus;

    User::WaitForNRequest(statuses, num);

    bool result = false;

    // By timeout?
    if (timerStatus != KRequestPending) {
        Q_ASSERT(selectForRead);
        *selectForRead = false;
        Q_ASSERT(selectForWrite);
        *selectForWrite = false;
    } else {
        selectTimer.Cancel();
        User::WaitForRequest(timerStatus);

        // By read?
        if (readStatus != KRequestPending) {
            Q_ASSERT(selectForRead);
            *selectForRead = true;
        }

        // By write?
        if (writeStatus != KRequestPending) {
            Q_ASSERT(selectForWrite);
            *selectForWrite = true;
        }

        if (checkRead)
            descriptor.NotifyDataAvailableCancel();
        if (checkWrite)
            descriptor.NotifyOutputEmptyCancel();

        result = true;
    }
    return result;
}

QString SerialPortPrivate::portNameToSystemLocation(const QString &port)
{
    // Port name is equval to port systemLocation.
    return port;
}

QString SerialPortPrivate::portNameFromSystemLocation(const QString &location)
{
    // Port name is equval to port systemLocation.
    return location;
}

struct RatePair
{
    qint32 rate;    // The numerical value of baud rate.
    qint32 setting; // The OS-specific code of baud rate.
    bool operator<(const RatePair &other) const { return rate < other.rate; }
    bool operator==(const RatePair &other) const { return setting == other.setting; }
};

// This table contains correspondences standard pairs values of
// baud rates that are defined in files
// - d32comm.h for Symbian^3
// - d32public.h for Symbian SR1
static const RatePair standardRatesTable[] =
{
    { 50, EBps50 },
    { 75, EBps75 },
    { 110, EBps110},
    { 134, EBps134 },
    { 150, EBps150 },
    { 300, EBps300 },
    { 600, EBps600 },
    { 1200, EBps1200 },
    { 1800, EBps1800 },
    { 2000, EBps2000 },
    { 2400, EBps2400 },
    { 3600, EBps3600 },
    { 4800, EBps4800 },
    { 7200, EBps7200 },
    { 9600, EBps9600 },
    { 19200, EBps19200 },
    { 38400, EBps38400 },
    { 57600, EBps57600 },
    { 115200, EBps115200 },
    { 230400, EBps230400 },
    { 460800, EBps460800 },
    { 576000, EBps576000 },
    { 921600, EBps921600 },
    { 1152000, EBps1152000 },
    //{ 1843200, EBps1843200 }, only for Symbian SR1
    { 4000000, EBps4000000 }
};

static const RatePair *standardRatesTable_end =
        standardRatesTable + sizeof(standardRatesTable)/sizeof(*standardRatesTable);

qint32 SerialPortPrivate::rateFromSetting(qint32 setting)
{
    const RatePair rp = { 0, setting };
    const RatePair *ret = qFind(standardRatesTable, standardRatesTable_end, rp);
    return ret != standardRatesTable_end ? ret->rate : 0;
}

qint32 SerialPortPrivate::settingFromRate(qint32 rate)
{
    const RatePair rp = { rate, 0 };
    const RatePair *ret = qBinaryFind(standardRatesTable, standardRatesTable_end, rp);
    return ret != standardRatesTable_end ? ret->setting : 0;
}

QList<qint32> SerialPortPrivate::standardRates()
{
    QList<qint32> ret;
    for (const RatePair *it = standardRatesTable; it != standardRatesTable_end; ++it)
        ret.append(it->rate);
    return ret;
}

QT_END_NAMESPACE_SERIALPORT
