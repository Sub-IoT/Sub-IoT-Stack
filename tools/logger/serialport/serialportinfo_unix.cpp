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
#include "ttylocker_unix_p.h"
#include "serialport_unix_p.h"
#include <QtCore/qfile.h>

#ifndef Q_OS_MAC

#if defined (Q_OS_LINUX) && defined (HAVE_LIBUDEV)
extern "C"
{
#include <libudev.h>
}
#else
#include <QtCore/qdir.h>
#include <QtCore/qstringlist.h>
#endif

#endif // Q_OS_MAC

QT_BEGIN_NAMESPACE_SERIALPORT

#ifndef Q_OS_MAC

#if defined (Q_OS_LINUX) && defined (HAVE_LIBUDEV)

// White list for devices without a parent
static const QString rfcommDeviceName(QLatin1String("rfcomm"));

#else

static QStringList generateFiltersOfDevices()
{
    QStringList l;

#  ifdef Q_OS_LINUX
    l << QLatin1String("ttyS*")    // Standart UART 8250 and etc.
      << QLatin1String("ttyUSB*")  // Usb/serial converters PL2303 and etc.
      << QLatin1String("ttyACM*")  // CDC_ACM converters (i.e. Mobile Phones).
      << QLatin1String("ttyGS*")   // Gadget serial device (i.e. Mobile Phones with gadget serial driver).
      << QLatin1String("ttyMI*")   // MOXA pci/serial converters.
      << QLatin1String("rfcomm*"); // Bluetooth serial device.
#  elif defined (Q_OS_FREEBSD)
    l << QLatin1String("cu*");
#  else
    // Here for other *nix OS.
#  endif

    return l;
}

inline QStringList& filtersOfDevices()
{
    static QStringList l = generateFiltersOfDevices();
    return l;
}

#endif

QList<SerialPortInfo> SerialPortInfo::availablePorts()
{
    QList<SerialPortInfo> ports;

#if defined (Q_OS_LINUX) && defined (HAVE_LIBUDEV)

    struct ::udev *udev = ::udev_new();
    if (udev) {

        struct ::udev_enumerate *enumerate =
                ::udev_enumerate_new(udev);

        if (enumerate) {

            ::udev_enumerate_add_match_subsystem(enumerate, "tty");
            ::udev_enumerate_scan_devices(enumerate);

            struct ::udev_list_entry *devices =
                    ::udev_enumerate_get_list_entry(enumerate);

            struct ::udev_list_entry *dev_list_entry;
            udev_list_entry_foreach(dev_list_entry, devices) {

                struct ::udev_device *dev =
                        ::udev_device_new_from_syspath(udev,
                                                       ::udev_list_entry_get_name(dev_list_entry));

                if (dev) {

                    SerialPortInfo info;

                    info.d_ptr->device =
                            QLatin1String(::udev_device_get_devnode(dev));
                    info.d_ptr->portName =
                            QLatin1String(::udev_device_get_sysname(dev));

                    struct ::udev_device *parentdev = ::udev_device_get_parent(dev);

                    bool canAppendToList = true;

                    if (parentdev) {

                        QLatin1String subsys(::udev_device_get_subsystem(parentdev));

                        if (subsys == QLatin1String("usb-serial")
                                || subsys == QLatin1String("usb")) { // USB bus type
                            // Append this devices and try get additional information about them.
                            info.d_ptr->description = QString(
                                    QLatin1String(::udev_device_get_property_value(dev,
                                                                                   "ID_MODEL"))).replace('_', ' ');
                            info.d_ptr->manufacturer = QString(
                                    QLatin1String(::udev_device_get_property_value(dev,
                                                                                   "ID_VENDOR"))).replace('_', ' ');
                            info.d_ptr->vendorIdentifier =
                                    QLatin1String(::udev_device_get_property_value(dev,
                                                                                   "ID_VENDOR_ID"));
                            info.d_ptr->productIdentifier =
                                    QLatin1String(::udev_device_get_property_value(dev,
                                                                                   "ID_MODEL_ID"));
                        } else if (subsys == QLatin1String("pnp")) { // PNP bus type
                            // Append this device.
                            // FIXME: How to get additional information about serial devices
                            // with this subsystem?
                        } else if (subsys == QLatin1String("platform")) { // Platform 'pseudo' bus for legacy device.
                            // Skip this devices because this type of subsystem does
                            // not include a real physical serial device.
                            canAppendToList = false;
                        } else { // Others types of subsystems.
                            // Append this devices because we believe that any other types of
                            // subsystems provide a real serial devices. For example, for devices
                            // such as ttyGSx, its driver provide an empty subsystem name, but it
                            // devices is a real physical serial devices.
                            // FIXME: How to get additional information about serial devices
                            // with this subsystems?
                        }
                    } else { // Devices without a parent
                        if (info.d_ptr->portName.startsWith(rfcommDeviceName)) { // Bluetooth device
                            bool ok;
                            // Check for an unsigned decimal integer at the end of the device name: "rfcomm0", "rfcomm15"
                            // devices with negative and invalid numbers in the name are rejected
                            int portNumber = info.d_ptr->portName.mid(rfcommDeviceName.length()).toInt(&ok);

                            if (!ok || (portNumber < 0) || (portNumber > 255)) {
                                canAppendToList = false;
                            }
                        } else {
                            canAppendToList = false;
                        }
                    }

                    if (canAppendToList)
                        ports.append(info);

                    ::udev_device_unref(dev);
                }

            }

            ::udev_enumerate_unref(enumerate);
        }

        ::udev_unref(udev);
    }

#elif defined (Q_OS_FREEBSD) && defined (HAVE_LIBUSB)
    // TODO: Implement me.
#else

    QDir devDir(QLatin1String("/dev"));
    if (devDir.exists()) {

        devDir.setNameFilters(filtersOfDevices());
        devDir.setFilter(QDir::Files | QDir::System | QDir::NoSymLinks);

        QStringList foundDevices; // Found devices list.

        foreach (const QFileInfo &fi, devDir.entryInfoList()) {
            QString s = fi.absoluteFilePath();
            if (!foundDevices.contains(s)) {
                foundDevices.append(s);

                SerialPortInfo info;

                info.d_ptr->device = s;
                info.d_ptr->portName = SerialPortPrivate::portNameFromSystemLocation(s);

                // Get description, manufacturer, vendor identifier, product
                // identifier are not supported.

                ports.append(info);

            }
        }
    }

#endif

    return ports;
}

#endif // Q_OS_MAC

// common part

QList<qint32> SerialPortInfo::standardRates()
{
    return SerialPortPrivate::standardRates();
}

bool SerialPortInfo::isBusy() const
{
    bool currentPid = false;
    return TtyLocker::isLocked(portName().toLocal8Bit().constData(), &currentPid);
}

bool SerialPortInfo::isValid() const
{
    QFile f(systemLocation());
    return f.exists();
}

QT_END_NAMESPACE_SERIALPORT
