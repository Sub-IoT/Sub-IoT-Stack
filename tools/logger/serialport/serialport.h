/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <scapig2@yandex.ru>
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

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QtCore/qiodevice.h>

#include "serialport-global.h"

QT_BEGIN_NAMESPACE_SERIALPORT

class SerialPortInfo;
class SerialPortPrivate;

class Q_SERIALPORT_EXPORT SerialPort : public QIODevice
{
    Q_OBJECT

    //Q_PROPERTY(qint32 rate READ rate WRITE setRate)
    //Q_PROPERTY(DataBits dataBits READ dataBits WRITE setDataBits)
    //Q_PROPERTY(Parity parity READ parity WRITE setParity)
    //Q_PROPERTY(StopBits stopBits READ stopBits WRITE setStopBits)
    //Q_PROPERTY(FlowControl flowControl READ flowControl WRITE setFlowControl)
    //Q_PROPERTY(DataErrorPolicy dataErrorPolicy READ dataErrorPolicy WRITE setDataErrorPolicy)
    //Q_PROPERTY(bool dtr READ dtr WRITE setDtr)
    //Q_PROPERTY(bool rts READ rts WRITE setRts)
    //Q_PROPERTY(PortError error READ error RESET unsetError)
    //Q_PROPERTY(bool restoreSettingsOnClose READ restoreSettingsOnClose WRITE setRestoreSettingsOnClose)

    Q_ENUMS( Directions Rate DataBits Parity StopBits FlowControl Lines DataErrorPolicy PortError )

public:

    enum Direction  {
        Input = 1,
        Output = 2,
        AllDirections = Input | Output
    };
    Q_DECLARE_FLAGS(Directions, Direction)

    enum Rate {
        Rate1200 = 1200,
        Rate2400 = 2400,
        Rate4800 = 4800,
        Rate9600 = 9600,
        Rate19200 = 19200,
        Rate38400 = 38400,
        Rate57600 = 57600,
        Rate115200 = 115200,
        UnknownRate = -1
    };

    enum DataBits {
        Data5 = 5,
        Data6 = 6,
        Data7 = 7,
        Data8 = 8,
        UnknownDataBits = -1
    };

    enum Parity {
        NoParity = 0,
        EvenParity = 2,
        OddParity = 3,
        SpaceParity = 4,
        MarkParity = 5,
        UnknownParity = -1
    };

    enum StopBits {
        OneStop = 1,
        OneAndHalfStop = 3,
        TwoStop = 2,
        UnknownStopBits = -1
    };

    enum FlowControl {
        NoFlowControl,
        HardwareControl,
        SoftwareControl,
        UnknownFlowControl = -1
    };

    enum Line {
        Le = 0x01,
        Dtr = 0x02,
        Rts = 0x04,
        St = 0x08,
        Sr = 0x10,
        Cts = 0x20,
        Dcd = 0x40,
        Ri = 0x80,
        Dsr = Le
    };
    Q_DECLARE_FLAGS(Lines, Line)

    enum DataErrorPolicy {
        SkipPolicy,
        PassZeroPolicy,
        IgnorePolicy,
        StopReceivingPolicy,
        UnknownPolicy = -1
    };

    enum PortError {
        NoError,
        NoSuchDeviceError,
        PermissionDeniedError,
        DeviceAlreadyOpenedError,
        DeviceIsNotOpenedError,
        ParityError,
        FramingError,
        BreakConditionError,
        IoError,
        UnsupportedPortOperationError,
        UnknownPortError
    };

    explicit SerialPort(QObject *parent = 0);
    explicit SerialPort(const QString &name, QObject *parent = 0);
    explicit SerialPort(const SerialPortInfo &info, QObject *parent = 0);
    virtual ~SerialPort();

    void setPort(const QString &port);
    void setPort(const SerialPortInfo &info);
    QString portName() const;

    virtual bool open(OpenMode mode);
    virtual void close();

    void setRestoreSettingsOnClose(bool restore);
    bool restoreSettingsOnClose() const;

    bool setRate(qint32 rate, Directions dir = AllDirections);
    qint32 rate(Directions dir = AllDirections) const;

    bool setDataBits(DataBits dataBits);
    DataBits dataBits() const;

    bool setParity(Parity parity);
    Parity parity() const;

    bool setStopBits(StopBits stopBits);
    StopBits stopBits() const;

    bool setFlowControl(FlowControl flow);
    FlowControl flowControl() const;

    bool dtr() const;
    bool rts() const;

    Lines lines() const;

    bool flush();
    bool clear(Directions dir = AllDirections);
    virtual bool atEnd() const;

    bool setDataErrorPolicy(DataErrorPolicy policy = IgnorePolicy);
    DataErrorPolicy dataErrorPolicy() const;

    PortError error() const;
    void unsetError();

    qint64 readBufferSize() const;
    void setReadBufferSize(qint64 size);

    virtual bool isSequential() const;

    virtual qint64 bytesAvailable() const;
    virtual qint64 bytesToWrite() const;
    virtual bool canReadLine() const;

    virtual bool waitForReadyRead(int msecs);
    virtual bool waitForBytesWritten(int msecs);

public Q_SLOTS:
    bool setDtr(bool set);
    bool setRts(bool set);
    bool sendBreak(int duration = 0);
    bool setBreak(bool set = true);
    bool clearBreak(bool clear = true);

protected:
    virtual qint64 readData(char *data, qint64 maxSize);
    virtual qint64 readLineData(char *data, qint64 maxSize);
    virtual qint64 writeData(const char *data, qint64 maxSize);

private:
    SerialPortPrivate * const d_ptr;

    Q_DECLARE_PRIVATE(SerialPort)
    Q_DISABLE_COPY(SerialPort)
};

inline bool SerialPort::clearBreak(bool clear)
{ return setBreak(!clear); }

Q_DECLARE_OPERATORS_FOR_FLAGS(SerialPort::Directions)
Q_DECLARE_OPERATORS_FOR_FLAGS(SerialPort::Lines)

QT_END_NAMESPACE_SERIALPORT

#endif // SERIALPORT_H
