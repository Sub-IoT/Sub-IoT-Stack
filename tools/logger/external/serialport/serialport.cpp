/****************************************************************************
**
** Copyright (C) 2011-2012 Denis Shienkov <scapig2@yandex.ru>
** Copyright (C) 2011 Sergey Belyashov <Sergey.Belyashov@gmail.com>
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

#include "serialport.h"
#include "serialportinfo.h"

#ifdef Q_OS_WIN
#include "serialport_win_p.h"
#elif defined (Q_OS_SYMBIAN)
#include "serialport_symbian_p.h"
#elif defined (Q_OS_UNIX)
#include "serialport_unix_p.h"
#else
#error Unsupported OS
#endif

#ifndef SERIALPORT_BUFFERSIZE
#  define SERIALPORT_BUFFERSIZE 16384
#endif

QT_BEGIN_NAMESPACE_SERIALPORT

SerialPortPrivateData::SerialPortPrivateData(SerialPort *q)
    : readBufferMaxSize(0)
    , readBuffer(SERIALPORT_BUFFERSIZE)
    , writeBuffer(SERIALPORT_BUFFERSIZE)
    , portError(SerialPort::NoError)
    , inputRate(0)
    , outputRate(0)
    , dataBits(SerialPort::UnknownDataBits)
    , parity(SerialPort::UnknownParity)
    , stopBits(SerialPort::UnknownStopBits)
    , flow(SerialPort::UnknownFlowControl)
    , policy(SerialPort::IgnorePolicy)
    , restoreSettingsOnClose(true)
    , q_ptr(q)
{
}

int SerialPortPrivateData::timeoutValue(int msecs, int elapsed)
{
    if (msecs == -1)
        return msecs;
    msecs -= elapsed;
    return qMax(msecs, 0);
}

/*!
    \class SerialPort

    \brief The SerialPort class provides functions to access
    serial ports.

    \reentrant
    \ingroup serialport-main
    \inmodule QtAddOnSerialPort
    \since 5.0

    This class resembles the functionality and behavior of the QAbstractSocket
    class in many aspects, for instance the I/O operations, the implementation
    of the wait methods, the internal architecture and so forth. Certain
    SerialPort method implementations were taken directly from QAbstractSocket
    with only minor changes.

    The features of the implementation and the conduct of the class are
    listed below:

    \list
    \o Provides only common functionality which includes
    configuring, I/O data stream, get and set control signals of the
    RS-232 lines.
    \o Does not support for terminal features as echo, control CR/LF and so
    forth.
    \o Always works in binary mode.
    \o Does not support the native ability for configuring timeouts
    and delays while reading.
    \o Does not provide tracking and notification when the state
    of RS-232 lines was changed.
    \endlist

    To get started with the SerialPort class, first create an object of
    that.

    Then, call the setPort() method in order to assign the object with the name
    of the desired serial port (which has to be present in the system).
    The name has to follow a certain format, which is platform dependent.

    The helper class SerialPortInfo allows the enumeration of all the serial
    ports in the system. This is useful to obtain the correct serial port name.

    The SerialPortInfo class can also be used as an input parameter for the
    setPort() method (to retrieve the currently assigned name, use the
    portName() method).

    After that, the serial port can be opened in read-only (r/o), write-only
    (w/o) or read-write (r/w) mode using the open() method.

    Note: The serial port is always opened with exclusive access
    (i.e. no other process or thread can access an already opened serial port).

    Having successfully opened, the SerialPort determines its current
    configuration and initializes itself to that. To access the current
    configuration use the rate(), dataBits(), parity(), stopBits(), and
    flowControl() methods.

    If these settings are satisfying, the I/O operation can be proceed with.
    Otherwise the port can be reconfigured to the desired setting using the
    setRate(), setDataBits(), setParity(), setStopBits(), and setFlowControl()
    methods.

    Read or write the data by calling read() or write(). Alternatively the
    readLine() and readAll() convenience methods can also be invoked. The
    SerialPort class also inherits the getChar(), putChar(), and ungetChar()
    methods from the QIODevice class. Those methods work on single bytes. The
    bytesWritten() signal is emitted when data has been written to the serial
    port. Note that, Qt does not limit the write buffer size, which can be
    monitored by listening to this signal.

    The readyRead() signal is emitted every time a new chunk of data
    has arrived. The bytesAvailable() method then returns the number of bytes
    that are available for reading. Typically, the readyRead() signal would be
    connected to a slot and all data available could be read in there.

    If not all the data is read at once, the remaining data will
    still be available later. Any new incoming data will be
    appended to the SerialPort's internal read buffer. In order to limit the
    size of the read buffer, call setReadBufferSize().

    The status of the control lines is determined with the dtr(), rts(), and
    lines() methods. To change the control line status, use the setDtr(), and
    setRts() methods.

    To close the serial port, call the close() method. After all the pending data
    has been written to the serial port, the SerialPort class actually closes
    the descriptor.

    SerialPort provides a set of functions that suspend the calling
    thread until certain signals are emitted. These functions can be
    used to implement blocking serial ports:

    \list
    \o waitForReadyRead() blocks until new data is available for
    reading.

    \o waitForBytesWritten() blocks until one payload of data has been
    written to the serial port.
    \endlist

    See the following example:

    \code
     int numRead = 0, numReadTotal = 0;
     char buffer[50];

     forever {
         numRead  = serial.read(buffer, 50);

         // Do whatever with the array

         numReadTotal += numRead;
         if (numRead == 0 && !serial.waitForReadyRead())
             break;
     }
    \endcode

    If \l{QIODevice::}{waitForReadyRead()} returns false, the
    connection has been closed or an error has occurred.

    Programming with a blocking serial port is radically different from
    programming with a non-blocking serial port. A blocking serial port
    does not require an event loop and typically leads to simpler code.
    However, in a GUI application, blocking serial port should only be
    used in non-GUI threads, to avoid freezing the user interface.

    See the \l examples/terminal and \l examples/blockingterminal
    examples for an overview of both approaches.

    The use of blocking functions is discouraged together with signals. One of
    the two possibilities should be used.

    The SerialPort class can be used with QTextStream and QDataStream's stream
    operators (operator<<() and operator>>()). There is one issue to be aware
    of, though: make sure that enough data is available before attempting to
    read by using the operator>>() overloaded operator.

    \sa SerialPortInfo
*/

/*!
    \enum SerialPort::Direction

    This enum describes the possible directions of the data transmission.
    Note: This enumeration is used for setting the baud rate of the device
    separately for each direction in case some operating systems (i.e. POSIX-like).

    \value Input            Input direction.
    \value Output           Output direction.
    \value AllDirections    Simultaneously in two directions.
*/

/*!
    \enum SerialPort::Rate

    This enum describes the baud rate which the communication device operates
    with. Note: only the most common standard rates are listed in this enum.

    \value Rate1200     1200 baud.
    \value Rate2400     2400 baud.
    \value Rate4800     4800 baud.
    \value Rate9600     9600 baud.
    \value Rate19200    19200 baud.
    \value Rate38400    38400 baud.
    \value Rate57600    57600 baud.
    \value Rate115200   115200 baud.
    \value UnknownRate  Unknown baud.

    \sa setRate(), rate()
*/

/*!
    \enum SerialPort::DataBits

    This enum describes the number of data bits used.

    \value Data5            Five bits.
    \value Data6            Six bits.
    \value Data7            Seven bits
    \value Data8            Eight bits.
    \value UnknownDataBits  Unknown number of bits.

    \sa setDataBits(), dataBits()
*/

/*!
    \enum SerialPort::Parity

    This enum describes the parity scheme used.

    \Value NoParity No parity.
    \value EvenParity Even parity.
    \value OddParity Odd parity.
    \value SpaceParity Space parity.
    \value MarkParity Mark parity.
    \value UnknownParity Unknown parity.

    \sa setParity(), parity()
*/

/*!
    \enum SerialPort::StopBits

    This enum describes the number of stop bits used.

    \value OneStop 1 stop bit.
    \value OneAndHalfStop 1.5 stop bits.
    \value TwoStop 2 stop bits.
    \value UnknownStopBits Unknown number of stop bit.

    \sa setStopBits(), stopBits()
*/

/*!
    \enum SerialPort::FlowControl

    This enum describes the flow control used.

    \value NoFlowControl No flow control.
    \value HardwareControl Hardware flow control (RTS/CTS).
    \value SoftwareControl Software flow control (XON/XOFF).
    \value UnknownFlowControl Unknown flow control.

    \sa setFlowControl(), flowControl()
*/

/*!
    \enum SerialPort::Line

    This enum describes the possible RS-232 pinout signals.

    \value Le DSR (data set ready/line enable).
    \value Dtr DTR (data terminal ready).
    \value Rts RTS (request to send).
    \value St Secondary TXD (transmit).
    \value Sr Secondary RXD (receive).
    \value Cts CTS (clear to send).
    \value Dcd DCD (data carrier detect).
    \value Ri RNG (ring).
    \value Dsr DSR (data set ready).

    \sa lines(), setDtr(), setRts(), dtr(), rts()
*/

/*!
    \enum SerialPort::DataErrorPolicy

    This enum describes the policies for the received symbols
    while parity errors were detected.

    \value SkipPolicy           Skips the bad character.
    \value PassZeroPolicy       Replaces bad character to zero.
    \value IgnorePolicy         Ignores the error for a bad character.
    \value StopReceivingPolicy  Stops data reception on error.
    \value UnknownPolicy        Unknown policy.

    \sa setDataErrorPolicy(), dataErrorPolicy()
*/

/*!
    \enum SerialPort::PortError

    This enum describes the errors that may be returned by the error()
    method.

    \value NoError              No error occurred.
    \value NoSuchDeviceError    An error occurred while attempting to
                                open an non-existing device.
    \value PermissionDeniedError An error occurred while attempting to
           open an already opened device by another process or a user not
           having enough permission and credentials to open.
    \value DeviceAlreadyOpenedError An error occurred while attempting to
           open an already opened device in this object.
    \value DeviceIsNotOpenedError An error occurred while attempting to
           control a device still closed.
    \value ParityError Parity error detected by the hardware while reading data.
    \value FramingError Framing error detected by the hardware while reading data.
    \value BreakConditionError Break condition detected by the hardware on
           the input line.
    \value IoError An I/O error occurred while reading or writing the data.
    \value UnsupportedPortOperationError The requested device operation is
           not supported or prohibited by the running operating system.
    \value UnknownPortError An unidentified error occurred.

    \sa error(), unsetError()
*/



/*!
    Constructs a new serial port object with the given \a parent.
*/
SerialPort::SerialPort(QObject *parent)
    : QIODevice(parent)
    , d_ptr(new SerialPortPrivate(this))
{}

/*!
    Constructs a new serial port object with the given \a parent
    to represent the serial port with the specified \a name.

    The name should have a specific format; see the setPort() method.
*/
SerialPort::SerialPort(const QString &name, QObject *parent)
    : QIODevice(parent)
    , d_ptr(new SerialPortPrivate(this))
{
    setPort(name);
}

/*!
    Constructs a new serial port object with the given \a parent
    to represent the serial port with the specified a helper
    class \a info.
*/
SerialPort::SerialPort(const SerialPortInfo &info, QObject *parent)
    : QIODevice(parent)
    , d_ptr(new SerialPortPrivate(this))
{
    setPort(info);
}

/*!
    Closes the serial port, if neccessary, and then destroys object.
*/
SerialPort::~SerialPort()
{
    /**/
    close();
    delete d_ptr;
}

/*!
    Sets the \a name of the port. The name may be in any format;
    either short, or also as system location (with all the prefixes and
    postfixed). As a result, this name will be automatically written
    and converted into an internal variable as system location.

    \sa portName(), SerialPortInfo
*/
void SerialPort::setPort(const QString &name)
{
    Q_D(SerialPort);
    d->systemLocation = SerialPortPrivate::portNameToSystemLocation(name);
}

/*!
    Sets the port stored in the serial port info instance \a info.

    \sa portName(), SerialPortInfo
*/
void SerialPort::setPort(const SerialPortInfo &info)
{
    Q_D(SerialPort);
    d->systemLocation = SerialPortPrivate::portNameToSystemLocation(info.systemLocation());
}

/*!
    Returns the name set by setPort() or to the SerialPort constructors.
    This name is short, i.e. it extract and convert out from the internal
    variable system location of the device. Conversion algorithm is
    platform specific:
    \table
    \header
        \o Platform
        \o Brief Description
    \row
        \o Windows
        \o Removes the prefix "\\\\.\\" from the system location
           and returns the remainder of the string.
    \row
        \o Windows CE
        \o Removes the postfix ":" from the system location
           and returns the remainder of the string.
    \row
        \o Symbian
        \o Returns the system location as it is,
           as it is equivalent to the port name.
    \row
        \o GNU/Linux
        \o Removes the prefix "/dev/" from the system location
           and returns the remainder of the string.
    \row
        \o Mac OSX
        \o Removes the prefix "/dev/cu." and "/dev/tty." from the
           system location and returns the remainder of the string.
    \row
        \o Other *nix
        \o The same as for GNU/Linux.
    \endtable

    \sa setPort(), SerialPortInfo::portName()
*/
QString SerialPort::portName() const
{
    Q_D(const SerialPort);
    return SerialPortPrivate::portNameFromSystemLocation(d->systemLocation);
}

/*! \reimp
    Opens the serial port using OpenMode \a mode, and then returns true if
    successful; otherwise returns false with and sets an error code which can be
    obtained by calling the error() method.

    \warning The \a mode has to be QIODevice::ReadOnly, QIODevice::WriteOnly,
    or QIODevice::ReadWrite. This may also have additional flags, such as
    QIODevice::Unbuffered. Other modes are unsupported.

    \sa QIODevice::OpenMode, setPort()
*/
bool SerialPort::open(OpenMode mode)
{
    Q_D(SerialPort);

    if (isOpen()) {
        d->portError = SerialPort::DeviceAlreadyOpenedError;
        return false;
    }

    // Define while not supported modes.
    static const OpenMode unsupportedModes = Append | Truncate | Text | Unbuffered;
    if ((mode & unsupportedModes) || mode == NotOpen) {
        d->portError = SerialPort::UnsupportedPortOperationError;
        return false;
    }

    unsetError();
    if (d->open(mode)) {
        QIODevice::open(mode);
        return true;
    }
    return false;
}

/*! \reimp
    Calls SerialPort::flush() and closes the serial port.
    Errors from flush are ignored.

    \sa QIODevice::close()
*/
void SerialPort::close()
{
    Q_D(SerialPort);
    if (!isOpen()) {
        d->portError = SerialPort::DeviceIsNotOpenedError;
        return;
    }

    QIODevice::close();
    d->close();
}

/*!
    Sets or clears the flag \a restore, which allows to restore the
    previous settings while closing the serial port. If this flag
    is true, the settings will be restored; otherwise not.
    The default state of the SerialPort class is configured to restore the
    settings.

    \sa restoreSettingsOnClose()
*/
void SerialPort::setRestoreSettingsOnClose(bool restore)
{
    Q_D( SerialPort);
    d->restoreSettingsOnClose = restore;
}

/*!
    Returns the current status of the restore flag settings on
    closing the port. The default SerialPort is configured to
    restore the settings.

    \sa setRestoreSettingsOnClose()
*/
bool SerialPort::restoreSettingsOnClose() const
{
    Q_D(const SerialPort);
    return d->restoreSettingsOnClose;
}

/*!
    Sets the desired data rate \a rate for a given direction \a dir. If
    successful, returns true; otherwise returns false and sets an error code
    which can be obtained by calling error(). To set the baud rate, use the
    enumeration SerialPort::Rate or any positive qint32 value.

    \warning For OS Windows, Windows CE, Symbian supported only
    AllDirections flag.

    \sa rate()
*/
bool SerialPort::setRate(qint32 rate, Directions dir)
{
    Q_D(SerialPort);
    if (d->setRate(rate, dir)) {
        if (dir & SerialPort::Input)
            d->inputRate = rate;
        if (dir & SerialPort::Output)
            d->outputRate = rate;
        return true;
    }
    return false;
}

/*!
    Returns the current baud rate of the chosen direction \a dir.

    \warning Returns equal rate in any direction for Operating Systems as
    Windows, Windows CE, and Symbian.

    \sa setRate()
*/
qint32 SerialPort::rate(Directions dir) const
{
    Q_D(const SerialPort);
    if (dir == SerialPort::AllDirections)
        return d->inputRate == d->outputRate ?
                    d->inputRate : SerialPort::UnknownRate;
    return dir & SerialPort::Input ? d->inputRate : d->outputRate;
}

/*!
    Sets the desired number of data bits \a dataBits in a frame.
    If successful, returns true; otherwise returns false and sets an error
    code which can be obtained by calling the error() method.

    \sa dataBits()
*/
bool SerialPort::setDataBits(DataBits dataBits)
{
    Q_D(SerialPort);
    if (d->setDataBits(dataBits)) {
        d->dataBits = dataBits;
        return true;
    }
    return false;
}

/*!
    Returns the current number of data bits in a frame.

    \sa setDataBits()
*/
SerialPort::DataBits SerialPort::dataBits() const
{
    Q_D(const SerialPort);
    return d->dataBits;
}

/*!
    Sets the desired parity \a parity checking mode.
    If successful, returns true; otherwise returns false and sets an error
    code which can be obtained by calling the error() method.

    \sa parity()
*/
bool SerialPort::setParity(Parity parity)
{
    Q_D(SerialPort);
    if (d->setParity(parity)) {
        d->parity = parity;
        return true;
    }
    return false;
}

/*!
    Returns the current parity checking mode.

    \sa setParity()
*/
SerialPort::Parity SerialPort::parity() const
{
    Q_D(const SerialPort);
    return d->parity;
}

/*!
    Sets the desired number of stop bits \a stopBits in a frame. If successful,
    returns true; otherwise returns false and sets an error code which can be
    obtained by calling the error() method.

    \sa stopBits()
*/
bool SerialPort::setStopBits(StopBits stopBits)
{
    Q_D(SerialPort);
    if (d->setStopBits(stopBits)) {
        d->stopBits = stopBits;
        return true;
    }
    return false;
}

/*!
    Returns the current number of stop bits.

    \sa setStopBits()
*/
SerialPort::StopBits SerialPort::stopBits() const
{
    Q_D(const SerialPort);
    return d->stopBits;
}

/*!
    Sets the desired number flow control mode \a flow.
    If successful, returns true; otherwise returns false and sets an error
    code which can be obtained by calling the error() method.

    \sa flowControl()
*/
bool SerialPort::setFlowControl(FlowControl flow)
{
    Q_D(SerialPort);
    if (d->setFlowControl(flow)) {
        d->flow = flow;
        return true;
    }
    return false;
}

/*!
    Returns the current flow control mode.

    \sa setFlowControl()
*/
SerialPort::FlowControl SerialPort::flowControl() const
{
    Q_D(const SerialPort);
    return d->flow;
}

/*!
    Returns the current state of the line signal DTR.
    If the signal state high, the return true; otherwise returns false;

    \sa lines()
*/
bool SerialPort::dtr() const
{
    Q_D(const SerialPort);
    return d->lines() & SerialPort::Dtr;
}

/*!
    Returns the current state of the line signal RTS.
    If the signal state high, the return true; otherwise returns false;

    \sa lines()
*/
bool SerialPort::rts() const
{
    Q_D(const SerialPort);
    return d->lines() & SerialPort::Rts;
}

/*!
    Returns the bitmap states of the line signals.
    From this result, it is possible to allocate the state of the
    desired signal by applying a mask "AND", where the mask is
    the desired enumeration value from SerialPort::Lines.

    \sa dtr(), rts(), setDtr(), setRts()
*/
SerialPort::Lines SerialPort::lines() const
{
    Q_D(const SerialPort);
    return d->lines();
}

/*!
    This function writes as much as possible from the internal write
    buffer to the underlying serial port without blocking. If any data
    was written, this function returns true; otherwise returns false.

    Call this function for sending the buffered data immediately to the serial
    port. The number of bytes successfully written depends on the operating
    system. In most cases, this function does not need to be called, because the
    SerialPort class will start sending data automatically once control is
    returned to the event loop. In the absence of an event loop, call
    waitForBytesWritten() instead.

    \sa write(), waitForBytesWritten()
*/
bool SerialPort::flush()
{
    Q_D(SerialPort);
    return d->flush();
}

/*!
    Discards all characters from the output or input buffer, depending on
    a given direction \a dir. Including clear an internal class buffers and
    the UART (driver) buffers. Also terminate pending read or write operations.
    If successful, returns true; otherwise returns false.
*/
bool SerialPort::clear(Directions dir)
{
    Q_D(SerialPort);
    if (dir & Input)
        d->readBuffer.clear();
    if (dir & Output)
        d->writeBuffer.clear();
    return d->clear(dir);
}

/*! \reimp

    Returns true if no more data is currently available for reading; otherwise
    returns false.

    This function is most commonly used when reading data from the
    serial port in a loop. For example:

    \code
    // This slot is connected to SerialPort::readyRead()
    void SerialPortClass::readyReadSlot()
    {
        while (!port.atEnd()) {
            QByteArray data = port.read(100);
            ....
        }
    }
    \endcode

     \sa bytesAvailable(), readyRead()
 */
bool SerialPort::atEnd() const
{
    Q_D(const SerialPort);
    return QIODevice::atEnd() && (!isOpen() || d->readBuffer.isEmpty());
}

/*!
    Sets the error policy \a policy process received character in
    the case of parity error detection. If successful, returns
    true; otherwise returns false. The default policy set is IgnorePolicy.

    \sa dataErrorPolicy()
*/
bool SerialPort::setDataErrorPolicy(DataErrorPolicy policy)
{
    Q_D(SerialPort);
    const bool ret = d->policy == policy || d->setDataErrorPolicy(policy);
    if (ret)
        d->policy = policy;
    return ret;
}

/*!
    Returns current error policy.

    \sa setDataErrorPolicy()
*/
SerialPort::DataErrorPolicy SerialPort::dataErrorPolicy() const
{
    Q_D(const SerialPort);
    return d->policy;
}

/*!
    Returns the serial port's error status.

    The I/O device status returns an error code. For example, if open()
    returns false, or a read/write operation returns -1, this function can
    be called to find out the reason why the operation failed.

    \sa unsetError()
*/
SerialPort::PortError SerialPort::error() const
{
    Q_D(const SerialPort);
    return d->portError;
}

/*!
    Clears the error code to SerialPort::NoError.

    \sa error()
*/
void SerialPort::unsetError()
{
    Q_D(SerialPort);
    d->portError = SerialPort::NoError;
}

/*!
    Returns the size of the internal read buffer. This limits the
    amount of data that the client can receive before calling the read()
    or readAll() methods.

    A read buffer size of 0 (the default) means that the buffer has
    no size limit, ensuring that no data is lost.

    \sa setReadBufferSize(), read()
*/
qint64 SerialPort::readBufferSize() const
{
    Q_D(const SerialPort);
    return d->readBufferMaxSize;
}

/*!
    Sets the size of SerialPort's internal read buffer to be \a
    size bytes.

    If the buffer size is limited to a certain size, SerialPort
    will not buffer more than this size of data. Exceptionally, a buffer
    size of 0 means that the read buffer is unlimited and all
    incoming data is buffered. This is the default.

    This option is useful if the data is only read at certain points
    in time (for instance in a real-time streaming application) or if the serial
    port should be protected against receiving too much data, which may
    eventually causes that the application runs out of memory.

    \sa readBufferSize(), read()
*/
void SerialPort::setReadBufferSize(qint64 size)
{
    Q_D(SerialPort);

    if (d->readBufferMaxSize == size)
        return;
    d->readBufferMaxSize = size;
}

/*! \reimp
    Always returns true. The serial port is a sequential device.
*/
bool SerialPort::isSequential() const
{
    return true;
}

/*! \reimp
    Returns the number of incoming bytes that are waiting to be read.

    \sa bytesToWrite(), read()
*/
qint64 SerialPort::bytesAvailable() const
{
    Q_D(const SerialPort);
    return d->readBuffer.size() + QIODevice::bytesAvailable();
}

/*! \reimp
    Returns the number of bytes that are waiting to be written. The
    bytes are written when control goes back to the event loop or
    when flush() is called.

    \sa bytesAvailable(), flush()
*/
qint64 SerialPort::bytesToWrite() const
{
    Q_D(const SerialPort);
    return d->writeBuffer.size() + QIODevice::bytesToWrite();
}

/*! \reimp
    Returns true if a line of data can be read from the serial port;
    otherwise returns false.

    \sa readLine()
*/
bool SerialPort::canReadLine() const
{
    Q_D(const SerialPort);
    const bool hasLine = d->readBuffer.canReadLine();
    return hasLine || QIODevice::canReadLine();
}

/*! \reimp
    This function blocks until new data is available for reading and the
    \l{QIODevice::}{readyRead()} signal has been emitted. The function
    will timeout after \a msecs milliseconds.

    The function returns true if the readyRead() signal is emitted and
    there is new data available for reading; otherwise it returns false
    (if an error occurred or the operation timed out).

    \sa waitForBytesWritten()
*/
bool SerialPort::waitForReadyRead(int msecs)
{
    Q_D(SerialPort);
    return d->waitForReadyRead(msecs);
}

/*! \reimp
*/
bool SerialPort::waitForBytesWritten(int msecs)
{
    Q_D(SerialPort);
    return d->waitForBytesWritten(msecs);
}

/*!
    Sets the desired state of the line signal DTR,
    depending on the flag \a set. If successful, returns true;
    otherwise returns false.

    If the flag is true then the DTR signal is set to high; otherwise low.

    \sa lines(), dtr()
*/
bool SerialPort::setDtr(bool set)
{
    Q_D(SerialPort);
    return d->setDtr(set);
}

/*!
    Sets the desired state of the line signal RTS,
    depending on the flag \a set. If successful, returns true;
    otherwise returns false.

    If the flag is true then the RTS signal is set to high; otherwise low.

    \sa lines(), rts()
*/
bool SerialPort::setRts(bool set)
{
    Q_D(SerialPort);
    return d->setRts(set);
}

/*!
    Sends a continuous stream of zero bits during a specified period
    of time \a duration in msec if the terminal is using asynchronous
    serial data. If successful, returns true; otherwise returns false.

    If the duration is zero then zero bits are transmitted by at least
    0.25 seconds, but no more than 0.5 seconds.

    If the duration is non zero then zero bits are transmitted within a certain
    period of time depending on the implementation.

    \sa setBreak(), clearBreak()
*/
bool SerialPort::sendBreak(int duration)
{
    Q_D(SerialPort);
    return d->sendBreak(duration);
}

/*!
    Controls the signal break, depending on the flag \a set.
    If successful, returns true; otherwise returns false.

    If \a set is true then enables the break transmission; otherwise disables.

    \sa clearBreak(), sendBreak()
*/
bool SerialPort::setBreak(bool set)
{
    Q_D(SerialPort);
    return d->setBreak(set);
}

/*! \reimp
*/
qint64 SerialPort::readData(char *data, qint64 maxSize)
{
    Q_D(SerialPort);
    return d->readFromBuffer(data, maxSize);
}

/*! \reimp
*/
qint64 SerialPort::readLineData(char *data, qint64 maxSize)
{
    return QIODevice::readLineData(data, maxSize);
}

/*! \reimp
*/
qint64 SerialPort::writeData(const char *data, qint64 maxSize)
{
    Q_D(SerialPort);
    return d->writeToBuffer(data, maxSize);
}

/*!
    \fn bool SerialPort::clearBreak(bool clear)

    Controls the signal break, depending on the flag \a clear.
    If successful, returns true; otherwise returns false.

    If clear is false then enables the break transmission; otherwise disables.

    \sa setBreak(), sendBreak()
*/

#include "moc_serialport.cpp"

QT_END_NAMESPACE_SERIALPORT
