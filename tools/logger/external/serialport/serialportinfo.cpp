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
#include "serialport.h"

QT_BEGIN_NAMESPACE_SERIALPORT


/*!
    \class SerialPortInfo

    \brief The SerialPortInfo class provides information about
    existing serial ports.

    \ingroup serialport-main
    \inmodule QtAddOnSerialPort
    \since 5.0

    Use the static functions to generate a list of SerialPortInfo
    objects. Each SerialPortInfo object in the list represents a single
    serial port and can be queried for the port name, system location,
    description, and manufacturer. The SerialPortInfo class can also be
    used as an input parameter for the setPort() method of the SerialPort
    class.

    \sa SerialPort
*/

/*!
    Constructs an empty SerialPortInfo object.

    \sa isNull()
*/
SerialPortInfo::SerialPortInfo()
    : d_ptr(new SerialPortInfoPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/
SerialPortInfo::SerialPortInfo(const SerialPortInfo &other)
    : d_ptr(other.d_ptr ? new SerialPortInfoPrivate(*other.d_ptr) : 0)
{
}

/*!
    Constructs a SerialPortInfo object from serial \a port.
*/
SerialPortInfo::SerialPortInfo(const SerialPort &port)
    : d_ptr(new SerialPortInfoPrivate)
{
    foreach (const SerialPortInfo &info, availablePorts()) {
        if (port.portName() == info.portName()) {
            *this = info;
            break;
        }
    }
}

/*!
    Constructs a SerialPortInfo object from serial port \a name.

    This constructor finds the relevant serial port among the available ones
    according to the port name \a name, and constructs the serial port info
    instance for that port.
*/
SerialPortInfo::SerialPortInfo(const QString &name)
    : d_ptr(new SerialPortInfoPrivate)
{
    foreach (const SerialPortInfo &info, availablePorts()) {
        if (name == info.portName()) {
            *this = info;
            break;
        }
    }
}

/*!
    Destroys the SerialPortInfo object. References to the values in the
    object become invalid.
*/
SerialPortInfo::~SerialPortInfo()
{
}

/*! \fn void SerialPortInfo::swap(SerialPortInfo &other)

    Swaps SerialPortInfo \a other with this SerialPortInfo. This operation is
    very fast and never fails.
*/
void SerialPortInfo::swap(SerialPortInfo &other)
{
    d_ptr.swap(other.d_ptr);
}

/*!
    Sets the SerialPortInfo object to be equal to \a other.
*/
SerialPortInfo& SerialPortInfo::operator=(const SerialPortInfo &other)
{
    SerialPortInfo(other).swap(*this);
    return *this;
}

/*!
    Returns the name of the serial port.
*/
QString SerialPortInfo::portName() const
{
    Q_D(const SerialPortInfo);
    return !d ? QString() : d->portName;
}

/*!
    Returns the system location of the serial port.
*/
QString SerialPortInfo::systemLocation() const
{
    Q_D(const SerialPortInfo);
    return !d ? QString() : d->device;
}

/*!
    Returns the description string of the serial port,
    if available; otherwise returns an empty string.
*/
QString SerialPortInfo::description() const
{
    Q_D(const SerialPortInfo);
    return !d ? QString() : d->description;
}

/*!
    Returns the manufacturer string of the serial port,
    if available; otherwise returns an empty string.
*/
QString SerialPortInfo::manufacturer() const
{
    Q_D(const SerialPortInfo);
    return !d ? QString() : d->manufacturer;
}

/*!
    Returns the vendor identifier string of the serial
    port in hexadecimal format, if available; otherwise
    returns an empty string.
*/
QString SerialPortInfo::vendorIdentifier() const
{
    Q_D(const SerialPortInfo);
    return !d ? QString() : d->vendorIdentifier;
}

/*!
    Returns the product identifier string of the serial
    port in hexadecimal format, if available; otherwise
    returns an empty string.
*/
QString SerialPortInfo::productIdentifier() const
{
    Q_D(const SerialPortInfo);
    return !d ? QString() : d->productIdentifier;
}

/*!
    \fn bool SerialPortInfo::isNull() const

    Returns whether this SerialPortInfo object holds a
    serial port definition.
*/

/*!
    \fn bool SerialPortInfo::isBusy() const

    Returns true if serial port is busy;
    otherwise returns false.
*/

/*!
    \fn bool SerialPortInfo::isValid() const

    Returns true if serial port is present on system;
    otherwise returns false.
*/

/*!
    \fn QList<qint32> SerialPortInfo::standardRates()

    Returns a list of available standard baud rates supported by
    the current serial port.
*/

/*!
    \fn QList<SerialPortInfo> SerialPortInfo::availablePorts()

    Returns a list of available serial ports on the system.
*/

QT_END_NAMESPACE_SERIALPORT
