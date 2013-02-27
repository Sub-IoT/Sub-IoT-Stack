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

#ifndef SERIALPORTINFO_H
#define SERIALPORTINFO_H

#include <QtCore/qlist.h>
#include <QtCore/qscopedpointer.h>

#include "serialport-global.h"

QT_BEGIN_NAMESPACE_SERIALPORT

class SerialPort;
class SerialPortInfoPrivate;
class SerialInfoPrivateDeleter;

class Q_SERIALPORT_EXPORT SerialPortInfo
{
    Q_DECLARE_PRIVATE(SerialPortInfo)
public:
    SerialPortInfo();
    SerialPortInfo(const SerialPortInfo &other);
    SerialPortInfo(const SerialPort &port);
    SerialPortInfo(const QString &name);
    ~SerialPortInfo();

    SerialPortInfo& operator=(const SerialPortInfo &other);
    void swap(SerialPortInfo &other);

    QString portName() const;
    QString systemLocation() const;
    QString description() const;
    QString manufacturer() const;
    QString vendorIdentifier() const;
    QString productIdentifier() const;

    bool isNull() const;
    bool isBusy() const;
    bool isValid() const;

    static QList<qint32> standardRates();
    static QList<SerialPortInfo> availablePorts();

private:
    QScopedPointer<SerialPortInfoPrivate, SerialInfoPrivateDeleter> d_ptr;
};

inline bool SerialPortInfo::isNull() const
{ return !d_ptr; }

QT_END_NAMESPACE_SERIALPORT

#endif // SERIALPORTINFO_H
