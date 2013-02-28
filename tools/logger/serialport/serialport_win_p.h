/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <scapig@yandex.ru>
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

#ifndef SERIALPORT_WIN_P_H
#define SERIALPORT_WIN_P_H

#include "serialport_p.h"

#include <qt_windows.h>

#ifndef Q_OS_WINCE
class QWinEventNotifier;
#include <QtCore/qhash.h>
#else
class QThread;
#include <QtCore/qmutex.h>
#endif

QT_BEGIN_NAMESPACE_SERIALPORT

#ifndef Q_OS_WINCE
class AbstractOverlappedEventNotifier;
#endif

class SerialPortPrivate : public SerialPortPrivateData
{
public:
    SerialPortPrivate(SerialPort *q);

    bool open(QIODevice::OpenMode mode);
    void close();

    SerialPort::Lines lines() const;

    bool setDtr(bool set);
    bool setRts(bool set);

    bool flush();
    bool clear(SerialPort::Directions dir);

    bool sendBreak(int duration);
    bool setBreak(bool set);

    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;

    qint64 readFromBuffer(char *data, qint64 maxSize);
    qint64 writeToBuffer(const char *data, qint64 maxSize);

    bool waitForReadyRead(int msec);
    bool waitForBytesWritten(int msec);

    bool setRate(qint32 rate, SerialPort::Directions dir);
    bool setDataBits(SerialPort::DataBits dataBits);
    bool setParity(SerialPort::Parity parity);
    bool setStopBits(SerialPort::StopBits stopBits);
    bool setFlowControl(SerialPort::FlowControl flowControl);
    bool setDataErrorPolicy(SerialPort::DataErrorPolicy policy);

    bool processIoErrors();
#ifndef Q_OS_WINCE
    bool startAsyncRead();
    bool startAsyncWrite(int maxSize = INT_MAX);
    bool completeAsyncRead(DWORD numberOfBytes);
    bool completeAsyncWrite(DWORD numberOfBytes);
    AbstractOverlappedEventNotifier *lookupFreeWriteCompletionNotifier();
    AbstractOverlappedEventNotifier *lookupCommEventNotifier();
    AbstractOverlappedEventNotifier *lookupReadCompletionNotifier();
#else
    bool notifyRead();
    bool notifyWrite(int maxSize = INT_MAX);
#endif

    static QString portNameToSystemLocation(const QString &port);
    static QString portNameFromSystemLocation(const QString &location);

    static qint32 rateFromSetting(qint32 setting);
    static qint32 settingFromRate(qint32 rate);

    static QList<qint32> standardRates();

    DCB currentDcb;
    DCB restoredDcb;
    COMMTIMEOUTS currentCommTimeouts;
    COMMTIMEOUTS restoredCommTimeouts;
    HANDLE descriptor;
    bool flagErrorFromCommEvent;

#ifndef Q_OS_WINCE
    QHash<HANDLE, AbstractOverlappedEventNotifier *> notifiers;
    qint64 actualReadBufferSize;
    qint64 actualWriteBufferSize;
    qint64 acyncWritePosition;
    bool readyReadEmitted;
    bool writeSequenceStarted;
#else
    QThread *eventNotifier;
    QMutex settingsChangeMutex;
#endif

private:
    bool updateDcb();
    bool updateCommTimeouts();

    void detectDefaultSettings();
    SerialPort::PortError decodeSystemError() const;

#ifndef Q_OS_WINCE
    bool waitAnyEvent(int msecs, bool *timedOut,
                      AbstractOverlappedEventNotifier **triggeredNotifier);
#else
    bool waitForReadOrWrite(bool *selectForRead, bool *selectForWrite,
                            bool checkRead, bool checkWrite,
                            int msecs, bool *timedOut);
#endif

};

QT_END_NAMESPACE_SERIALPORT

#endif // SERIALPORT_WIN_P_H
