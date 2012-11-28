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

#include "serialport_unix_p.h"
#include "ttylocker_unix_p.h"

#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef Q_OS_MAC
#if defined (MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
#include <IOKit/serial/ioss.h>
#endif
#endif

#include <QtCore/qelapsedtimer.h>

#include <QtCore/qsocketnotifier.h>

QT_BEGIN_NAMESPACE_SERIALPORT

class ReadNotifier : public QSocketNotifier
{
public:
    ReadNotifier(SerialPortPrivate *d, QObject *parent)
        : QSocketNotifier(d->descriptor, QSocketNotifier::Read, parent)
        , dptr(d)
    {}

protected:
    virtual bool event(QEvent *e) {
        bool ret = QSocketNotifier::event(e);
        if (ret)
            dptr->readNotification();
        return ret;
    }

private:
    SerialPortPrivate *dptr;
};

class WriteNotifier : public QSocketNotifier
{
public:
    WriteNotifier(SerialPortPrivate *d, QObject *parent)
        : QSocketNotifier(d->descriptor, QSocketNotifier::Write, parent)
        , dptr(d)
    {}

protected:
    virtual bool event(QEvent *e) {
        bool ret = QSocketNotifier::event(e);
        if (ret)
            dptr->writeNotification(SerialPortPrivateData::WriteChunkSize);
        return ret;
    }

private:
    SerialPortPrivate *dptr;
};

class ExceptionNotifier : public QSocketNotifier
{
public:
    ExceptionNotifier(SerialPortPrivate *d, QObject *parent)
        : QSocketNotifier(d->descriptor, QSocketNotifier::Exception, parent)
        , dptr(d)
    {}

protected:
    virtual bool event(QEvent *e) {
        bool ret = QSocketNotifier::event(e);
        if (ret)
            dptr->exceptionNotification();
        return ret;
    }

private:
    SerialPortPrivate *dptr;
};

SerialPortPrivate::SerialPortPrivate(SerialPort *q)
    : SerialPortPrivateData(q)
    , descriptor(-1)
    , isCustomRateSupported(false)
    , readNotifier(0)
    , writeNotifier(0)
    , exceptionNotifier(0)
    , readPortNotifierCalled(false)
    , readPortNotifierState(false)
    , readPortNotifierStateSet(false)
    , emittedReadyRead(false)
    , emittedBytesWritten(false)
{
}

bool SerialPortPrivate::open(QIODevice::OpenMode mode)
{
    QByteArray portName = portNameFromSystemLocation(systemLocation).toLocal8Bit();
    const char *ptr = portName.constData();

    bool byCurrPid = false;
    if (TtyLocker::isLocked(ptr, &byCurrPid)) {
        portError = SerialPort::PermissionDeniedError;
        return false;
    }

    int flags = O_NOCTTY | O_NONBLOCK;

    switch (mode & QIODevice::ReadWrite) {
    case QIODevice::WriteOnly:
        flags |= O_WRONLY;
        break;
    case QIODevice::ReadWrite:
        flags |= O_RDWR;
        break;
    default:
        flags |= O_RDONLY;
        break;
    }

    descriptor = ::open(systemLocation.toLocal8Bit().constData(), flags);

    if (descriptor == -1) {
        portError = decodeSystemError();
        return false;
    }

    ::fcntl(descriptor, F_SETFL, FNDELAY);

    TtyLocker::lock(ptr);
    if (!TtyLocker::isLocked(ptr, &byCurrPid)) {
        portError = SerialPort::PermissionDeniedError;
        return false;
    }

#ifdef TIOCEXCL
    ::ioctl(descriptor, TIOCEXCL);
#endif

    if (::tcgetattr(descriptor, &restoredTermios) == -1) {
        portError = decodeSystemError();
        return false;
    }

    currentTermios = restoredTermios;
    ::cfmakeraw(&currentTermios);
    currentTermios.c_cflag |= CLOCAL;
    currentTermios.c_cc[VTIME] = 0;
    currentTermios.c_cc[VMIN] = 0;

    if (mode & QIODevice::ReadOnly)
        currentTermios.c_cflag |= CREAD;

    if (!updateTermios())
        return false;

    setExceptionNotificationEnabled(true);

    if ((flags & O_WRONLY) == 0)
        setReadNotificationEnabled(true);

    detectDefaultSettings();
    return true;
}

void SerialPortPrivate::close()
{
    if (restoreSettingsOnClose) {
        ::tcsetattr(descriptor, TCSANOW, &restoredTermios);
#ifdef Q_OS_LINUX
        if (isCustomRateSupported)
            ::ioctl(descriptor, TIOCSSERIAL, &restoredSerialInfo);
#endif
    }

#ifdef TIOCNXCL
    ::ioctl(descriptor, TIOCNXCL);
#endif

    if (readNotifier) {
        readNotifier->setEnabled(false);
        readNotifier->deleteLater();
        readNotifier = 0;
    }

    if (writeNotifier) {
        writeNotifier->setEnabled(false);
        writeNotifier->deleteLater();
        writeNotifier = 0;
    }

    if (exceptionNotifier) {
        exceptionNotifier->setEnabled(false);
        exceptionNotifier->deleteLater();
        exceptionNotifier = 0;
    }

    ::close(descriptor);

    QByteArray portName = portNameFromSystemLocation(systemLocation).toLocal8Bit();
    const char *ptr = portName.constData();

    bool byCurrPid = false;
    if (TtyLocker::isLocked(ptr, &byCurrPid) && byCurrPid)
        TtyLocker::unlock(ptr);

    descriptor = -1;
    isCustomRateSupported = false;
}

SerialPort::Lines SerialPortPrivate::lines() const
{
    int arg = 0;
    SerialPort::Lines ret = 0;

    if (::ioctl(descriptor, TIOCMGET, &arg) == -1)
        return ret;

#ifdef TIOCM_LE
    if (arg & TIOCM_LE)
        ret |= SerialPort::Le;
#endif
#ifdef TIOCM_DTR
    if (arg & TIOCM_DTR)
        ret |= SerialPort::Dtr;
#endif
#ifdef TIOCM_RTS
    if (arg & TIOCM_RTS)
        ret |= SerialPort::Rts;
#endif
#ifdef TIOCM_ST
    if (arg & TIOCM_ST)
        ret |= SerialPort::St;
#endif
#ifdef TIOCM_SR
    if (arg & TIOCM_SR)
        ret |= SerialPort::Sr;
#endif
#ifdef TIOCM_CTS
    if (arg & TIOCM_CTS)
        ret |= SerialPort::Cts;
#endif
#ifdef TIOCM_CAR
    if (arg & TIOCM_CAR)
        ret |= SerialPort::Dcd;
#elif defined TIOCM_CD
    if (arg & TIOCM_CD)
        ret |= SerialPort::Dcd;
#endif
#ifdef TIOCM_RNG
    if (arg & TIOCM_RNG)
        ret |= SerialPort::Ri;
#elif defined TIOCM_RI
    if (arg & TIOCM_RI)
        ret |= SerialPort::Ri;
#endif
#ifdef TIOCM_DSR
    if (arg & TIOCM_DSR)
        ret |= SerialPort::Dsr;
#endif

    return ret;
}

bool SerialPortPrivate::setDtr(bool set)
{
    int status = TIOCM_DTR;
    return ::ioctl(descriptor, set ? TIOCMBIS : TIOCMBIC, &status) != -1;
}

bool SerialPortPrivate::setRts(bool set)
{
    int status = TIOCM_RTS;
    return ::ioctl(descriptor, set ? TIOCMBIS : TIOCMBIC, &status) != -1;
}

bool SerialPortPrivate::flush()
{
    return writeNotification() && (::tcdrain(descriptor) != -1);
}

bool SerialPortPrivate::clear(SerialPort::Directions dir)
{
    return ::tcflush(descriptor, (dir == SerialPort::AllDirections)
                     ? TCIOFLUSH : (dir & SerialPort::Input) ? TCIFLUSH : TCOFLUSH) != -1;
}

bool SerialPortPrivate::sendBreak(int duration)
{
    return ::tcsendbreak(descriptor, duration) != -1;
}

bool SerialPortPrivate::setBreak(bool set)
{
    return ::ioctl(descriptor, set ? TIOCSBRK : TIOCCBRK) != -1;
}

qint64 SerialPortPrivate::bytesAvailable() const
{
    int nbytes = 0;
#ifdef TIOCINQ
    if (::ioctl(descriptor, TIOCINQ, &nbytes) == -1)
        return -1;
#endif
    return nbytes;
}

qint64 SerialPortPrivate::bytesToWrite() const
{
    int nbytes = 0;
#ifdef TIOCOUTQ
    if (::ioctl(descriptor, TIOCOUTQ, &nbytes) == -1)
        return -1;
#endif
    return nbytes;
}

qint64 SerialPortPrivate::readFromBuffer(char *data, qint64 maxSize)
{
    if (readBuffer.isEmpty())
        return 0;

    if (maxSize == 1) {
        *data = readBuffer.getChar();
        if (readBuffer.isEmpty())
            setReadNotificationEnabled(true);
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

    if (!isReadNotificationEnabled())
        setReadNotificationEnabled(true);

    if (readSoFar > 0) {
        if (readBuffer.isEmpty())
            setReadNotificationEnabled(true);
        return readSoFar;
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

    const qint64 written = maxSize;

    if (!writeBuffer.isEmpty() && !isWriteNotificationEnabled())
        setWriteNotificationEnabled(true);

    return written;
}

bool SerialPortPrivate::waitForReadyRead(int msecs)
{
    QElapsedTimer stopWatch;

    stopWatch.start();

    do {
        bool readyToRead = false;
        bool readyToWrite = false;
        bool timedOut = false;
        if (!waitForReadOrWrite(&readyToRead, &readyToWrite, true, !writeBuffer.isEmpty(),
                                timeoutValue(msecs, stopWatch.elapsed()), &timedOut)) {
            // TODO: set error ?
            return false;
        }

        if (readyToRead) {
            if (readNotification())
                return true;
        }

        if (readyToWrite)
            writeNotification(WriteChunkSize);

    } while (msecs == -1 || timeoutValue(msecs, stopWatch.elapsed()) > 0);
    return false;
}

bool SerialPortPrivate::waitForBytesWritten(int msecs)
{
    if (writeBuffer.isEmpty())
        return false;

    QElapsedTimer stopWatch;

    stopWatch.start();

    forever {
        bool readyToRead = false;
        bool readyToWrite = false;
        bool timedOut = false;
        if (!waitForReadOrWrite(&readyToRead, &readyToWrite, true, !writeBuffer.isEmpty(),
                                timeoutValue(msecs, stopWatch.elapsed()), &timedOut)) {
            // TODO: set error ?
            return false;
        }

        if (readyToRead && !readNotification())
            return false;

        if (readyToWrite && writeNotification(WriteChunkSize))
            return true;
    }
    return false;
}

bool SerialPortPrivate::setRate(qint32 rate, SerialPort::Directions dir)
{
    bool ret = rate > 0;

    // prepare section

    if (ret) {
        const qint32 unixRate = SerialPortPrivate::settingFromRate(rate);
        if (unixRate > 0) {
            // try prepate to set standard baud rate
#ifdef Q_OS_LINUX
            // prepare to forcefully reset the custom mode
            if (isCustomRateSupported) {
                //currentSerialInfo.flags |= ASYNC_SPD_MASK;
                currentSerialInfo.flags &= ~(ASYNC_SPD_CUST /* | ASYNC_LOW_LATENCY*/);
                currentSerialInfo.custom_divisor = 0;
            }
#endif
            // prepare to set standard rate
            ret = !(((dir & SerialPort::Input) && ::cfsetispeed(&currentTermios, unixRate) < 0)
                    || ((dir & SerialPort::Output) && ::cfsetospeed(&currentTermios, unixRate) < 0));
        } else {
            // try prepate to set custom baud rate
#ifdef Q_OS_LINUX
            // prepare to forcefully set the custom mode
            if (isCustomRateSupported) {
                currentSerialInfo.flags &= ~ASYNC_SPD_MASK;
                currentSerialInfo.flags |= (ASYNC_SPD_CUST /* | ASYNC_LOW_LATENCY*/);
                currentSerialInfo.custom_divisor = currentSerialInfo.baud_base / rate;
                if (currentSerialInfo.custom_divisor == 0)
                    currentSerialInfo.custom_divisor = 1;
                // for custom mode needed prepare to set B38400 rate
                ret = (::cfsetspeed(&currentTermios, B38400) != -1);
            } else {
                ret = false;
            }
#elif defined(Q_OS_MAC)

#  if defined (MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
            // Starting with Tiger, the IOSSIOSPEED ioctl can be used to set arbitrary baud rates
            // other than those specified by POSIX. The driver for the underlying serial hardware
            // ultimately determines which baud rates can be used. This ioctl sets both the input
            // and output speed.
            ret = ::ioctl(descriptor, IOSSIOSPEED, &rate) != -1;
#  else
            // others MacOSX version, can't prepare to set custom rate
            ret = false;
#  endif

#else
            // others *nix OS, can't prepare to set custom rate
            ret = false;
#endif
        }
    }

    // finally section

#ifdef Q_OS_LINUX
    if (ret && isCustomRateSupported) // finally, set or reset the custom mode
        ret = ::ioctl(descriptor, TIOCSSERIAL, &currentSerialInfo) != -1;
#endif

    if (ret) // finally, set rate
        ret = updateTermios();
    else
        portError = decodeSystemError();
    return ret;
}

bool SerialPortPrivate::setDataBits(SerialPort::DataBits dataBits)
{
    currentTermios.c_cflag &= ~CSIZE;
    switch (dataBits) {
    case SerialPort::Data5:
        currentTermios.c_cflag |= CS5;
        break;
    case SerialPort::Data6:
        currentTermios.c_cflag |= CS6;
        break;
    case SerialPort::Data7:
        currentTermios.c_cflag |= CS7;
        break;
    case SerialPort::Data8:
        currentTermios.c_cflag |= CS8;
        break;
    default:
        currentTermios.c_cflag |= CS8;
        break;
    }
    return updateTermios();
}

bool SerialPortPrivate::setParity(SerialPort::Parity parity)
{
    currentTermios.c_iflag &= ~(PARMRK | INPCK);
    currentTermios.c_iflag |= IGNPAR;

    switch (parity) {

#ifdef CMSPAR
    // Here Installation parity only for GNU/Linux where the macro CMSPAR.
    case SerialPort::SpaceParity:
        currentTermios.c_cflag &= ~PARODD;
        currentTermios.c_cflag |= PARENB | CMSPAR;
        break;
    case SerialPort::MarkParity:
        currentTermios.c_cflag |= PARENB | CMSPAR | PARODD;
        break;
#endif
    case SerialPort::NoParity:
        currentTermios.c_cflag &= ~PARENB;
        break;
    case SerialPort::EvenParity:
        currentTermios.c_cflag &= ~PARODD;
        currentTermios.c_cflag |= PARENB;
        break;
    case SerialPort::OddParity:
        currentTermios.c_cflag |= PARENB | PARODD;
        break;
    default:
        currentTermios.c_cflag |= PARENB;
        currentTermios.c_iflag |= PARMRK | INPCK;
        currentTermios.c_iflag &= ~IGNPAR;
        break;
    }

    return updateTermios();
}

bool SerialPortPrivate::setStopBits(SerialPort::StopBits stopBits)
{
    switch (stopBits) {
    case SerialPort::OneStop:
        currentTermios.c_cflag &= ~CSTOPB;
        break;
    case SerialPort::TwoStop:
        currentTermios.c_cflag |= CSTOPB;
        break;
    default:
        currentTermios.c_cflag &= ~CSTOPB;
        break;
    }
    return updateTermios();
}

bool SerialPortPrivate::setFlowControl(SerialPort::FlowControl flow)
{
    switch (flow) {
    case SerialPort::NoFlowControl:
        currentTermios.c_cflag &= ~CRTSCTS;
        currentTermios.c_iflag &= ~(IXON | IXOFF | IXANY);
        break;
    case SerialPort::HardwareControl:
        currentTermios.c_cflag |= CRTSCTS;
        currentTermios.c_iflag &= ~(IXON | IXOFF | IXANY);
        break;
    case SerialPort::SoftwareControl:
        currentTermios.c_cflag &= ~CRTSCTS;
        currentTermios.c_iflag |= IXON | IXOFF | IXANY;
        break;
    default:
        currentTermios.c_cflag &= ~CRTSCTS;
        currentTermios.c_iflag &= ~(IXON | IXOFF | IXANY);
        break;
    }
    return updateTermios();
}

bool SerialPortPrivate::setDataErrorPolicy(SerialPort::DataErrorPolicy policy)
{
    tcflag_t parmrkMask = PARMRK;
#ifndef CMSPAR
    // in space/mark parity emulation also used PARMRK flag
    if (parity == SerialPort::SpaceParity
            || parity == SerialPort::MarkParity) {
        parmrkMask = 0;
    }
#endif //CMSPAR
    switch (policy) {
    case SerialPort::SkipPolicy:
        currentTermios.c_iflag &= ~parmrkMask;
        currentTermios.c_iflag |= IGNPAR | INPCK;
        break;
    case SerialPort::PassZeroPolicy:
        currentTermios.c_iflag &= ~(IGNPAR | parmrkMask);
        currentTermios.c_iflag |= INPCK;
        break;
    case SerialPort::IgnorePolicy:
        currentTermios.c_iflag &= ~INPCK;
        break;
    case SerialPort::StopReceivingPolicy:
        currentTermios.c_iflag &= ~IGNPAR;
        currentTermios.c_iflag |= parmrkMask | INPCK;
        break;
    default:
        currentTermios.c_iflag &= ~INPCK;
        break;
    }
    return updateTermios();
}

bool SerialPortPrivate::readNotification()
{
    // Prevent recursive calls
    if (readPortNotifierCalled) {
        if (!readPortNotifierStateSet) {
            readPortNotifierStateSet = true;
            readPortNotifierState = isReadNotificationEnabled();
            setReadNotificationEnabled(false);
        }
    }

    readPortNotifierCalled = true;

    // Always buffered, read data from the port into the read buffer
    qint64 newBytes = readBuffer.size();
    qint64 bytesToRead = policy == SerialPort::IgnorePolicy ? ReadChunkSize : 1;

    if (readBufferMaxSize && bytesToRead > (readBufferMaxSize - readBuffer.size())) {
        bytesToRead = readBufferMaxSize - readBuffer.size();
        if (bytesToRead == 0) {
            // Buffer is full. User must read data from the buffer
            // before we can read more from the port.
            return false;
        }
    }

    char *ptr = readBuffer.reserve(bytesToRead);
    const qint64 readBytes = readFromPort(ptr, bytesToRead);
    if (readBytes == -2) {
        // No bytes currently available for reading.
        readBuffer.chop(bytesToRead);
        return true;
    }
    readBuffer.chop(bytesToRead - qMax(readBytes, qint64(0)));

    newBytes = readBuffer.size() - newBytes;

    // If read buffer is full, disable the read port notifier.
    if (readBufferMaxSize && readBuffer.size() == readBufferMaxSize)
        setReadNotificationEnabled(false);

    // only emit readyRead() when not recursing, and only if there is data available
    const bool hasData = newBytes > 0;

    if (!emittedReadyRead && hasData) {
        emittedReadyRead = true;
        emit q_ptr->readyRead();
        emittedReadyRead = false;
    }

    if (!hasData)
        setReadNotificationEnabled(true);

    // reset the read port notifier state if we reentered inside the
    // readyRead() connected slot.
    if (readPortNotifierStateSet
            && readPortNotifierState != isReadNotificationEnabled()) {
        setReadNotificationEnabled(readPortNotifierState);
        readPortNotifierStateSet = false;
    }
    return true;
}

bool SerialPortPrivate::writeNotification(int maxSize)
{
    const int tmp = writeBuffer.size();

    if (writeBuffer.isEmpty()) {
        setWriteNotificationEnabled(false);
        return false;
    }

    int nextSize = qMin(writeBuffer.nextDataBlockSize(), maxSize);

    const char *ptr = writeBuffer.readPointer();

    // Attempt to write it chunk.
    qint64 written = writeToPort(ptr, nextSize);
    if (written < 0) {
        // TODO: set error?
        return false;
    }

    // Remove what we wrote so far.
    writeBuffer.free(written);
    if (written > 0) {
        // Don't emit bytesWritten() recursively.
        if (!emittedBytesWritten) {
            emittedBytesWritten = true;
            emit q_ptr->bytesWritten(written);
            emittedBytesWritten = false;
        }
    }

    if (writeBuffer.isEmpty())
        setWriteNotificationEnabled(false);

    return (writeBuffer.size() < tmp);
}

bool SerialPortPrivate::exceptionNotification()
{
    // FIXME:
    return false;
}

bool SerialPortPrivate::updateTermios()
{
    if (::tcsetattr(descriptor, TCSANOW, &currentTermios) == -1) {
        portError = decodeSystemError();
        return false;
    }
    return true;
}

void SerialPortPrivate::detectDefaultSettings()
{
    // Detect rate.
    const speed_t inputUnixRate = ::cfgetispeed(&currentTermios);
    const speed_t outputUnixRate = ::cfgetospeed(&currentTermios);
    bool isCustomRateCurrentSet = false;

#ifdef Q_OS_LINUX
    // try detect the ability to support custom rate
    isCustomRateSupported = ::ioctl(descriptor, TIOCGSERIAL, &currentSerialInfo) != -1
            && ::ioctl(descriptor, TIOCSSERIAL, &currentSerialInfo) != -1;

    if (isCustomRateSupported) {
        restoredSerialInfo = currentSerialInfo;

        // assume that the baud rate is a custom
        isCustomRateCurrentSet = inputUnixRate == B38400 && outputUnixRate == B38400;

        if (isCustomRateCurrentSet) {
            if ((currentSerialInfo.flags & ASYNC_SPD_CUST)
                    && currentSerialInfo.custom_divisor > 0) {

                // yes, speed is really custom
                inputRate = currentSerialInfo.baud_base / currentSerialInfo.custom_divisor;
                outputRate = inputRate;
            } else {
                // no, we were wrong and the speed is a standard 38400 baud
                isCustomRateCurrentSet = false;
            }
        }
    }
#else
    // other *nix
#endif
    if (!isCustomRateSupported || !isCustomRateCurrentSet) {
        inputRate = SerialPortPrivate::rateFromSetting(inputUnixRate);
        outputRate = SerialPortPrivate::rateFromSetting(outputUnixRate);
    }

    // Detect databits.
    switch (currentTermios.c_cflag & CSIZE) {
    case CS5:
        dataBits = SerialPort::Data5;
        break;
    case CS6:
        dataBits = SerialPort::Data6;
        break;
    case CS7:
        dataBits = SerialPort::Data7;
        break;
    case CS8:
        dataBits = SerialPort::Data8;
        break;
    default:
        dataBits = SerialPort::UnknownDataBits;
        break;
    }

    // Detect parity.
#ifdef CMSPAR
    if (currentTermios.c_cflag & CMSPAR) {
        parity = currentTermios.c_cflag & PARODD ?
                    SerialPort::MarkParity : SerialPort::SpaceParity;
    } else {
#endif
        if (currentTermios.c_cflag & PARENB) {
            parity = currentTermios.c_cflag & PARODD ?
                        SerialPort::OddParity : SerialPort::EvenParity;
        } else {
            parity = SerialPort::NoParity;
        }
#ifdef CMSPAR
    }
#endif

    // Detect stopbits.
    stopBits = currentTermios.c_cflag & CSTOPB ?
                SerialPort::TwoStop : SerialPort::OneStop;

    // Detect flow control.
    if ((!(currentTermios.c_cflag & CRTSCTS)) && (!(currentTermios.c_iflag & (IXON | IXOFF | IXANY))))
        flow = SerialPort::NoFlowControl;
    else if ((!(currentTermios.c_cflag & CRTSCTS)) && (currentTermios.c_iflag & (IXON | IXOFF | IXANY)))
        flow = SerialPort::SoftwareControl;
    else if ((currentTermios.c_cflag & CRTSCTS) && (!(currentTermios.c_iflag & (IXON | IXOFF | IXANY))))
        flow = SerialPort::HardwareControl;
    else
        flow = SerialPort::UnknownFlowControl;
}

SerialPort::PortError SerialPortPrivate::decodeSystemError() const
{
    SerialPort::PortError error;
    switch (errno) {
    case ENODEV:
        error = SerialPort::NoSuchDeviceError;
        break;
    case EACCES:
        error = SerialPort::PermissionDeniedError;
        break;
    case EBUSY:
        error = SerialPort::PermissionDeniedError;
        break;
    case ENOTTY:
        error = SerialPort::IoError;
        break;
    default:
        error = SerialPort::UnknownPortError;
        break;
    }
    return error;
}

bool SerialPortPrivate::isReadNotificationEnabled() const
{
    return readNotifier && readNotifier->isEnabled();
}

void SerialPortPrivate::setReadNotificationEnabled(bool enable)
{
    if (readNotifier) {
        readNotifier->setEnabled(enable);
    } else if (enable) {
        readNotifier = new ReadNotifier(this, q_ptr);
        readNotifier->setEnabled(true);
    }
}

bool SerialPortPrivate::isWriteNotificationEnabled() const
{
    return writeNotifier && writeNotifier->isEnabled();
}

void SerialPortPrivate::setWriteNotificationEnabled(bool enable)
{
    if (writeNotifier) {
        writeNotifier->setEnabled(enable);
    } else if (enable) {
        writeNotifier = new WriteNotifier(this, q_ptr);
        writeNotifier->setEnabled(true);
    }
}

bool SerialPortPrivate::isExceptionNotificationEnabled() const
{
    return exceptionNotifier && exceptionNotifier->isEnabled();
}

void SerialPortPrivate::setExceptionNotificationEnabled(bool enable)
{
    if (exceptionNotifier) {
        exceptionNotifier->setEnabled(enable);
    } else if (enable) {
        exceptionNotifier = new ExceptionNotifier(this, q_ptr);
        exceptionNotifier->setEnabled(true);
    }
}

bool SerialPortPrivate::waitForReadOrWrite(bool *selectForRead, bool *selectForWrite,
                                           bool checkRead, bool checkWrite,
                                           int msecs, bool *timedOut)
{
    Q_ASSERT(selectForRead);
    Q_ASSERT(selectForWrite);
    Q_ASSERT(timedOut);

    fd_set fdread;
    FD_ZERO(&fdread);
    if (checkRead)
        FD_SET(descriptor, &fdread);

    fd_set fdwrite;
    FD_ZERO(&fdwrite);
    if (checkWrite)
        FD_SET(descriptor, &fdwrite);

    struct timeval tv;
    tv.tv_sec = msecs / 1000;
    tv.tv_usec = (msecs % 1000) * 1000;

    int ret = ::select(descriptor + 1, &fdread, &fdwrite, 0, msecs < 0 ? 0 : &tv);
    if (ret < 0)
        return false;
    if (ret == 0) {
        *timedOut = true;
        return false;
    }

    *selectForRead = FD_ISSET(descriptor, &fdread);
    *selectForWrite = FD_ISSET(descriptor, &fdwrite);

    return ret;
}

qint64 SerialPortPrivate::readFromPort(char *data, qint64 maxSize)
{
    qint64 bytesRead = 0;
#if defined (CMSPAR)
    if (parity == SerialPort::NoParity
            || policy != SerialPort::StopReceivingPolicy) {
#else
    if (parity != SerialPort::MarkParity
            && parity != SerialPort::SpaceParity) {
#endif
        bytesRead = ::read(descriptor, data, maxSize);
    } else {// Perform parity emulation.
        bytesRead = readPerChar(data, maxSize);
    }

    // FIXME: Here 'errno' codes for sockets.
    // You need to replace the codes for the serial port.
    if (bytesRead < 0) {
        bytesRead = -1;
        switch (errno) {
#if EWOULDBLOCK-0 && EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
        case EAGAIN:
            // No data was available for reading.
            bytesRead = -2;
            break;
        case EBADF:
        case EINVAL:
        case EIO:
            break;
        case ECONNRESET:
            bytesRead = 0;
            break;
        default:
            break;
        }
    }
    return bytesRead;
}

qint64 SerialPortPrivate::writeToPort(const char *data, qint64 maxSize)
{
    qint64 bytesWritten = 0;
#if defined (CMSPAR)
    bytesWritten = ::write(descriptor, data, maxSize);
#else
    if (parity != SerialPort::MarkParity
            && parity != SerialPort::SpaceParity) {
        bytesWritten = ::write(descriptor, data, maxSize);
    } else {// Perform parity emulation.
        bytesWritten = writePerChar(data, maxSize);
    }
#endif

    // FIXME: Here 'errno' codes for sockets.
    // You need to replace the codes for the serial port.
    if (bytesWritten < 0) {
        switch (errno) {
        case EPIPE:
        case ECONNRESET:
            bytesWritten = -1;
            break;
        case EAGAIN:
            bytesWritten = 0;
            break;
        case EMSGSIZE:
            break;
        default:
            break;
        }
    }
    return bytesWritten;
}

static inline bool evenParity(quint8 c)
{
    c ^= c >> 4;        //(c7 ^ c3)(c6 ^ c2)(c5 ^ c1)(c4 ^ c0)
    c ^= c >> 2;        //[(c7 ^ c3)(c5 ^ c1)][(c6 ^ c2)(c4 ^ c0)]
    c ^= c >> 1;
    return c & 1;       //(c7 ^ c3)(c5 ^ c1)(c6 ^ c2)(c4 ^ c0)
}

#ifndef CMSPAR

qint64 SerialPortPrivate::writePerChar(const char *data, qint64 maxSize)
{
    qint64 ret = 0;
    quint8 const charMask = (0xFF >> (8 - dataBits));

    while (ret < maxSize) {

        bool par = evenParity(*data & charMask);
        // False if need EVEN, true if need ODD.
        par ^= parity == SerialPort::MarkParity;
        if (par ^ (currentTermios.c_cflag & PARODD)) { // Need switch parity mode?
            currentTermios.c_cflag ^= PARODD;
            flush(); //force sending already buffered data, because updateTermios() cleares buffers
            //todo: add receiving buffered data!!!
            if (!updateTermios())
                break;
        }

        int r = ::write(descriptor, data, 1);
        if (r < 0)
            return -1;
        if (r > 0) {
            data += r;
            ret += r;
        }
    }
    return ret;
}

#endif //CMSPAR

qint64 SerialPortPrivate::readPerChar(char *data, qint64 maxSize)
{
    qint64 ret = 0;
    quint8 const charMask = (0xFF >> (8 - dataBits));

    // 0 - prefix not started,
    // 1 - received 0xFF,
    // 2 - received 0xFF and 0x00
    int prefix = 0;
    while (ret < maxSize) {

        qint64 r = ::read(descriptor, data, 1);
        if (r < 0) {
            if (errno == EAGAIN) // It is ok for nonblocking mode.
                break;
            return -1;
        }
        if (r == 0)
            break;

        bool par = true;
        switch (prefix) {
        case 2: // Previously received both 0377 and 0.
            par = false;
            prefix = 0;
            break;
        case 1: // Previously received 0377.
            if (*data == '\0') {
                ++prefix;
                continue;
            }
            prefix = 0;
            break;
        default:
            if (*data == '\377') {
                prefix = 1;
                continue;
            }
            break;
        }
        // Now: par contains parity ok or error, *data contains received character
        par ^= evenParity(*data & charMask); //par contains parity bit value for EVEN mode
        par ^= (currentTermios.c_cflag & PARODD); //par contains parity bit value for current mode
        if (par ^ (parity == SerialPort::SpaceParity)) { //if parity error
            switch (policy) {
            case SerialPort::SkipPolicy:
                continue;       //ignore received character
            case SerialPort::StopReceivingPolicy:
                if (parity != SerialPort::NoParity)
                    portError = SerialPort::ParityError;
                else
                    portError = *data == '\0' ?
                                SerialPort::BreakConditionError : SerialPort::FramingError;
                return ++ret;   //abort receiving
                break;
            case SerialPort::UnknownPolicy:
                // Unknown error policy is used! Falling back to PassZeroPolicy
            case SerialPort::PassZeroPolicy:
                *data = '\0';   //replace received character by zero
                break;
            case SerialPort::IgnorePolicy:
                break;          //ignore error and pass received character
            }
        }
        ++data;
        ++ret;
    }
    return ret;
}

#ifdef Q_OS_MAC
static const QLatin1String defaultPathPrefix("/dev/cu.");
static const QLatin1String notUsedPathPrefix("/dev/tty.");
#else
static const QLatin1String defaultPathPrefix("/dev/");
#endif

QString SerialPortPrivate::portNameToSystemLocation(const QString &port)
{
    QString ret = port;

#ifdef Q_OS_MAC
    ret.remove(notUsedPathPrefix);
#endif

    if (!ret.contains(defaultPathPrefix))
        ret.prepend(defaultPathPrefix);
    return ret;
}

QString SerialPortPrivate::portNameFromSystemLocation(const QString &location)
{
    QString ret = location;

#ifdef Q_OS_MAC
    ret.remove(notUsedPathPrefix);
#endif

    ret.remove(defaultPathPrefix);
    return ret;
}

struct RatePair
{
    qint32 rate;    // The numerical value of baud rate.
    qint32 setting; // The OS-specific code of baud rate.
    bool operator<(const RatePair &other) const { return rate < other.rate; }
    bool operator==(const RatePair &other) const { return setting == other.setting; }
};

// This table contains correspondences standard pairs values of
// baud rates that are defined in file termios.h
static const RatePair standardRatesTable[] =
{
    #ifdef B50
    { 50, B50 },
    #endif
    #ifdef B75
    { 75, B75 },
    #endif
    #ifdef B110
    { 110, B110 },
    #endif
    #ifdef B134
    { 134, B134 },
    #endif
    #ifdef B150
    { 150, B150 },
    #endif
    #ifdef B200
    { 200, B200 },
    #endif
    #ifdef B300
    { 300, B300 },
    #endif
    #ifdef B600
    { 600, B600 },
    #endif
    #ifdef B1200
    { 1200, B1200 },
    #endif
    #ifdef B1800
    { 1800, B1800 },
    #endif
    #ifdef B2400
    { 2400, B2400 },
    #endif
    #ifdef B4800
    { 4800, B4800 },
    #endif
    #ifdef B9600
    { 9600, B9600 },
    #endif
    #ifdef B19200
    { 19200, B19200 },
    #endif
    #ifdef B38400
    { 38400, B38400 },
    #endif
    #ifdef B57600
    { 57600, B57600 },
    #endif
    #ifdef B115200
    { 115200, B115200 },
    #endif
    #ifdef B230400
    { 230400, B230400 },
    #endif
    #ifdef B460800
    { 460800, B460800 },
    #endif
    #ifdef B500000
    { 500000, B500000 },
    #endif
    #ifdef B576000
    { 576000, B576000 },
    #endif
    #ifdef B921600
    { 921600, B921600 },
    #endif
    #ifdef B1000000
    { 1000000, B1000000 },
    #endif
    #ifdef B1152000
    { 1152000, B1152000 },
    #endif
    #ifdef B1500000
    { 1500000, B1500000 },
    #endif
    #ifdef B2000000
    { 2000000, B2000000},
    #endif
    #ifdef B2500000
    { 2500000, B2500000 },
    #endif
    #ifdef B3000000
    { 3000000, B3000000 },
    #endif
    #ifdef B3500000
    { 3500000, B3500000 },
    #endif
    #ifdef B4000000
    { 4000000, B4000000 }
    #endif
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
