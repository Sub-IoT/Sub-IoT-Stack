/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <scapig@yandex.ru>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Copyright (C) 2012 Andre Hartmann <aha_1980@gmx.de>
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

#include "serialport_win_p.h"

#include <QtCore/qelapsedtimer.h>

#include <QtCore/qthread.h>
#include <QtCore/qtimer.h>

QT_BEGIN_NAMESPACE_SERIALPORT

class SerialPortPrivate;

class CommEventNotifier : public QThread
{
    Q_OBJECT
signals:
    void eventMask(quint32 mask);

public:
    CommEventNotifier(DWORD mask, SerialPortPrivate *d, QObject *parent)
        : QThread(parent), dptr(d), running(true) {
        connect(this, SIGNAL(eventMask(quint32)), this, SLOT(processNotification(quint32)));
        ::SetCommMask(dptr->descriptor, mask);
    }

    virtual ~CommEventNotifier() {
        running = false;
        ::SetCommMask(dptr->descriptor, 0);
        wait();
    }

protected:
    virtual void run() {
        DWORD mask = 0;
        while (running) {
            if (::WaitCommEvent(dptr->descriptor, &mask, FALSE)) {
                // Wait until complete the operation changes the port settings,
                // see updateDcb().
                dptr->settingsChangeMutex.lock();
                dptr->settingsChangeMutex.unlock();
                emit eventMask(quint32(mask));
            }
        }
    }

private slots:
    void processNotification(quint32 eventMask) {
        if (EV_ERR & eventMask)
            dptr->processIoErrors();
        if (EV_RXCHAR &eventMask)
            dptr->notifyRead();
        if (EV_TXEMPTY & eventMask)
            dptr->notifyWrite(SerialPortPrivateData::WriteChunkSize);
    }

private:
    SerialPortPrivate *dptr;
    mutable bool running;
};

class WaitCommEventBreaker : public QThread
{
    Q_OBJECT
public:
    WaitCommEventBreaker(HANDLE descriptor, int timeout, QObject *parent = 0)
        : QThread(parent), descriptor(descriptor), timeout(timeout), worked(false) {
        start();
    }

    virtual ~WaitCommEventBreaker() {
        stop();
        wait();
    }

    void stop() {
        exit(0);
    }

    bool isWorked() const {
        return worked;
    }

protected:
    void run() {
        QTimer timer;
        QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(processTimeout()), Qt::DirectConnection);
        timer.start(timeout);
        exec();
        worked = true;
    }

private slots:
    void processTimeout() {
        ::SetCommMask(descriptor, 0);
        stop();
    }

private:
    HANDLE descriptor;
    int timeout;
    mutable bool worked;
};

SerialPortPrivate::SerialPortPrivate(SerialPort *q)
    : SerialPortPrivateData(q)
    , descriptor(INVALID_HANDLE_VALUE)
    , flagErrorFromCommEvent(0)
    , eventNotifier(0)
{
}

bool SerialPortPrivate::open(QIODevice::OpenMode mode)
{
    DWORD desiredAccess = 0;
    DWORD eventMask = EV_ERR;

    if (mode & QIODevice::ReadOnly) {
        desiredAccess |= GENERIC_READ;
        eventMask |= EV_RXCHAR;
    }
    if (mode & QIODevice::WriteOnly) {
        desiredAccess |= GENERIC_WRITE;
        eventMask |= EV_TXEMPTY;
    }

    descriptor = ::CreateFile(reinterpret_cast<const wchar_t*>(systemLocation.utf16()),
                              desiredAccess, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (descriptor == INVALID_HANDLE_VALUE) {
        portError = decodeSystemError();
        return false;
    }

    if (!::GetCommState(descriptor, &restoredDcb)) {
        portError = decodeSystemError();
        return false;
    }

    currentDcb = restoredDcb;
    currentDcb.fBinary = true;
    currentDcb.fInX = false;
    currentDcb.fOutX = false;
    currentDcb.fAbortOnError = false;
    currentDcb.fNull = false;
    currentDcb.fErrorChar = false;

    if (!updateDcb())
        return false;

    if (!::GetCommTimeouts(descriptor, &restoredCommTimeouts)) {
        portError = decodeSystemError();
        return false;
    }

    ::memset(&currentCommTimeouts, 0, sizeof(currentCommTimeouts));
    currentCommTimeouts.ReadIntervalTimeout = MAXDWORD;

    if (!updateCommTimeouts())
        return false;

    eventNotifier = new QtAddOn::SerialPort::CommEventNotifier(eventMask, this, q_ptr);
    eventNotifier->start();

    detectDefaultSettings();
    return true;
}

void SerialPortPrivate::close()
{
    if (eventNotifier) {
        eventNotifier->deleteLater();
        eventNotifier = 0;
    }

    if (restoreSettingsOnClose) {
        ::SetCommState(descriptor, &restoredDcb);
        ::SetCommTimeouts(descriptor, &restoredCommTimeouts);
    }

    ::CloseHandle(descriptor);
    descriptor = INVALID_HANDLE_VALUE;
}

bool SerialPortPrivate::flush()
{
    return notifyWrite() && ::FlushFileBuffers(descriptor);
}

qint64 SerialPortPrivate::readFromBuffer(char *data, qint64 maxSize)
{
    if (readBuffer.isEmpty())
        return 0;

    if (maxSize == 1) {
        *data = readBuffer.getChar();
        return 1;
    }

    const qint64 bytesToRead = qMin(qint64(readBuffer.size()), maxSize);
    qint64 readSoFar = 0;
    while (readSoFar < bytesToRead) {
        const char *ptr = readBuffer.readPointer();
        const int bytesToReadFromThisBlock = qMin(int(bytesToRead - readSoFar),
                                                  readBuffer.nextDataBlockSize());
        ::memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
        readSoFar += bytesToReadFromThisBlock;
        readBuffer.free(bytesToReadFromThisBlock);
    }

    return readSoFar;
}

qint64 SerialPortPrivate::writeToBuffer(const char *data, qint64 maxSize)
{
    char *ptr = writeBuffer.reserve(maxSize);
    if (maxSize == 1)
        *ptr = *data;
    else
        ::memcpy(ptr, data, maxSize);

    return maxSize;
}

bool SerialPortPrivate::waitForReadyRead(int msec)
{
    if (!readBuffer.isEmpty())
        return true;

    QElapsedTimer stopWatch;

    stopWatch.start();

    forever {
        bool readyToRead = false;
        bool readyToWrite = false;
        bool timedOut = false;
        if (!waitForReadOrWrite(&readyToRead, &readyToWrite,
                                true, !writeBuffer.isEmpty(),
                                timeoutValue(msec, stopWatch.elapsed()),
                                &timedOut)) {
            return false;
        }
        if (readyToRead) {
            if (notifyRead())
                return true;
        }
        if (readyToWrite)
            notifyWrite(WriteChunkSize);
    }
    return false;
}

bool SerialPortPrivate::waitForBytesWritten(int msec)
{
    if (writeBuffer.isEmpty())
        return false;

    QElapsedTimer stopWatch;

    stopWatch.start();

    forever {
        bool readyToRead = false;
        bool readyToWrite = false;
        bool timedOut = false;
        if (!waitForReadOrWrite(&readyToRead, &readyToWrite,
                                true, !writeBuffer.isEmpty(),
                                timeoutValue(msec, stopWatch.elapsed()),
                                &timedOut)) {
            return false;
        }
        if (readyToRead) {
            if (!notifyRead())
                return false;
        }
        if (readyToWrite) {
            if (notifyWrite(WriteChunkSize))
                return true;
        }
    }
    return false;
}

bool SerialPortPrivate::notifyRead()
{
    DWORD bytesToRead = (policy == SerialPort::IgnorePolicy) ? ReadChunkSize : 1;

    if (readBufferMaxSize && bytesToRead > (readBufferMaxSize - readBuffer.size())) {
        bytesToRead = readBufferMaxSize - readBuffer.size();
        if (bytesToRead == 0) {
            // Buffer is full. User must read data from the buffer
            // before we can read more from the port.
            return false;
        }
    }

    char *ptr = readBuffer.reserve(bytesToRead);

    DWORD readBytes = 0;
    BOOL sucessResult = ::ReadFile(descriptor, ptr, bytesToRead, &readBytes, NULL);

    if (!sucessResult) {
        readBuffer.truncate(bytesToRead);
        return false;
    }

    readBuffer.truncate(readBytes);

    // Process emulate policy.
    if (flagErrorFromCommEvent) {
        flagErrorFromCommEvent = false;

        switch (policy) {
        case SerialPort::SkipPolicy:
            readBuffer.getChar();
            return true;
        case SerialPort::PassZeroPolicy:
            readBuffer.getChar();
            readBuffer.putChar('\0');
            break;
        case SerialPort::StopReceivingPolicy:
            // FIXME: Maybe need disable read notifier?
            break;
        default:
            break;
        }
    }

    if (readBytes > 0)
        emit q_ptr->readyRead();

    return true;
}

bool SerialPortPrivate::notifyWrite(int maxSize)
{
    int nextSize = qMin(writeBuffer.nextDataBlockSize(), maxSize);

    const char *ptr = writeBuffer.readPointer();

    DWORD bytesWritten = 0;
    if (!::WriteFile(descriptor, ptr, nextSize, &bytesWritten, NULL))
        return false;

    writeBuffer.free(bytesWritten);

    if (bytesWritten > 0)
        emit q_ptr->bytesWritten(bytesWritten);

    return true;
}

bool SerialPortPrivate::waitForReadOrWrite(bool *selectForRead, bool *selectForWrite,
                                           bool checkRead, bool checkWrite,
                                           int msecs, bool *timedOut)
{
    // FIXME: Here the situation is not properly handled with zero timeout:
    // breaker can work out before you call a method WaitCommEvent()
    // and so it will loop forever!
    WaitCommEventBreaker breaker(descriptor, qMax(msecs, 0));
    ::WaitCommEvent(descriptor, &eventMask, NULL);
    breaker.stop();

    if (breaker.isWorked())
        *timedOut = true;

    if (!breaker.isWorked()) {
        if (checkRead) {
            Q_ASSERT(selectForRead);
            *selectForRead = eventMask & EV_RXCHAR;
        }
        if (checkWrite) {
            Q_ASSERT(selectForWrite);
            *selectForWrite = eventMask & EV_TXEMPTY;
        }

        return true;
    }

    return false;
}

bool SerialPortPrivate::updateDcb()
{
    QMutexLocker locker(&settingsChangeMutex);

    DWORD eventMask = 0;
    // Save the event mask
    if (!::GetCommMask(descriptor, &eventMask))
        return false;

    // Break event notifier from WaitCommEvent
    ::SetCommMask(descriptor, 0);
    // Change parameters
    bool ret = ::SetCommState(descriptor, &currentDcb);
    if (!ret)
        portError = decodeSystemError();
    // Restore the event mask
    ::SetCommMask(descriptor, eventMask);

    return ret;
}

bool SerialPortPrivate::updateCommTimeouts()
{
    if (!::SetCommTimeouts(descriptor, &currentCommTimeouts)) {
        portError = decodeSystemError();
        return false;
    }
    return true;
}

static const QLatin1String defaultPathPostfix(":");

QString SerialPortPrivate::portNameToSystemLocation(const QString &port)
{
    QString ret = port;
    if (!ret.contains(defaultPathPostfix))
        ret.append(defaultPathPostfix);
    return ret;
}

QString SerialPortPrivate::portNameFromSystemLocation(const QString &location)
{
    QString ret = location;
    if (ret.contains(defaultPathPostfix))
        ret.remove(defaultPathPostfix);
    return ret;
}

#include "serialport_wince.moc"

QT_END_NAMESPACE_SERIALPORT
