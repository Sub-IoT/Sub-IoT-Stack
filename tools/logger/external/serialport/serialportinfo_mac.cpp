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

#include <sys/param.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h> // for kIOPropertyProductNameKey
#include <IOKit/usb/USB.h>
#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
#  include <IOKit/serial/ioss.h>
#endif
#include <IOKit/IOBSD.h>

QT_BEGIN_NAMESPACE_SERIALPORT

enum { MATCHING_PROPERTIES_COUNT = 6 };

QList<SerialPortInfo> SerialPortInfo::availablePorts()
{
    QList<SerialPortInfo> ports;

    int matchingPropertiesCounter = 0;


    ::CFMutableDictionaryRef matching = ::IOServiceMatching(kIOSerialBSDServiceValue);

    ::CFDictionaryAddValue(matching,
                           CFSTR(kIOSerialBSDTypeKey),
                           CFSTR(kIOSerialBSDAllTypes));

    io_iterator_t iter = 0;
    kern_return_t kr = ::IOServiceGetMatchingServices(kIOMasterPortDefault,
                                                      matching,
                                                      &iter);

    if (kr != kIOReturnSuccess)
        return ports;

    io_registry_entry_t service;

    while ((service = ::IOIteratorNext(iter))) {

        ::CFTypeRef device = 0;
        ::CFTypeRef portName = 0;
        ::CFTypeRef description = 0;
        ::CFTypeRef manufacturer = 0;
        ::CFTypeRef vendorIdentifier = 0;
        ::CFTypeRef productIdentifier = 0;

        io_registry_entry_t entry = service;

        // Find MacOSX-specific properties names.
        do {

            if (!device) {
                device =
                        ::IORegistryEntrySearchCFProperty(entry,
                                                          kIOServicePlane,
                                                          CFSTR(kIOCalloutDeviceKey),
                                                          kCFAllocatorDefault,
                                                          0);
                if (device)
                    ++matchingPropertiesCounter;
            }

            if (!portName) {
                portName =
                        ::IORegistryEntrySearchCFProperty(entry,
                                                          kIOServicePlane,
                                                          CFSTR(kIOTTYDeviceKey),
                                                          kCFAllocatorDefault,
                                                          0);
                if (portName)
                    ++matchingPropertiesCounter;
            }

            if (!description) {
                description =
                        ::IORegistryEntrySearchCFProperty(entry,
                                                          kIOServicePlane,
                                                          CFSTR(kIOPropertyProductNameKey),
                                                          kCFAllocatorDefault,
                                                          0);
                if (!description)
                    description =
                            ::IORegistryEntrySearchCFProperty(entry,
                                                              kIOServicePlane,
                                                              CFSTR(kUSBProductString),
                                                              kCFAllocatorDefault,
                                                              0);
                if (description)
                    ++matchingPropertiesCounter;
            }

            if (!manufacturer) {
                manufacturer =
                        ::IORegistryEntrySearchCFProperty(entry,
                                                          kIOServicePlane,
                                                          CFSTR(kUSBVendorString),
                                                          kCFAllocatorDefault,
                                                          0);
                if (manufacturer)
                    ++matchingPropertiesCounter;

            }

            if (!vendorIdentifier) {
                vendorIdentifier =
                        ::IORegistryEntrySearchCFProperty(entry,
                                                          kIOServicePlane,
                                                          CFSTR(kUSBVendorID),
                                                          kCFAllocatorDefault,
                                                          0);
                if (vendorIdentifier)
                    ++matchingPropertiesCounter;

            }

            if (!productIdentifier) {
                productIdentifier =
                        ::IORegistryEntrySearchCFProperty(entry,
                                                          kIOServicePlane,
                                                          CFSTR(kUSBProductID),
                                                          kCFAllocatorDefault,
                                                          0);
                if (productIdentifier)
                    ++matchingPropertiesCounter;

            }

            // If all matching properties is found, then force break loop.
            if (matchingPropertiesCounter == MATCHING_PROPERTIES_COUNT)
                break;

            kr = ::IORegistryEntryGetParentEntry(entry, kIOServicePlane, &entry);

        } while (kr == kIOReturnSuccess);

        (void) ::IOObjectRelease(entry);

        // Convert from MacOSX-specific properties to Qt4-specific.
        if (matchingPropertiesCounter > 0) {

            SerialPortInfo info;
            QByteArray buffer(MAXPATHLEN, 0);

            if (device) {
                if (::CFStringGetCString(CFStringRef(device),
                                         buffer.data(),
                                         buffer.size(),
                                         kCFStringEncodingUTF8)) {
                    info.d_ptr->device = QString(buffer);
                }
                ::CFRelease(device);
            }

            if (portName) {
                if (::CFStringGetCString(CFStringRef(portName),
                                         buffer.data(),
                                         buffer.size(),
                                         kCFStringEncodingUTF8)) {
                    info.d_ptr->portName = QString(buffer);
                }
                ::CFRelease(portName);
            }

            if (description) {
                ::CFStringGetCString(CFStringRef(description),
                                     buffer.data(),
                                     buffer.size(),
                                     kCFStringEncodingUTF8);

                info.d_ptr->description = QString(buffer);
                ::CFRelease(description);
            }

            if (manufacturer) {
                ::CFStringGetCString(CFStringRef(manufacturer),
                                     buffer.data(),
                                     buffer.size(),
                                     kCFStringEncodingUTF8);

                info.d_ptr->manufacturer = QString(buffer);
                ::CFRelease(manufacturer);
            }

            int value = 0;

            if (vendorIdentifier) {
                ::CFNumberGetValue(CFNumberRef(vendorIdentifier),
                                   kCFNumberIntType,
                                   &value);

                info.d_ptr->vendorIdentifier = QString::number(value, 16);
                ::CFRelease(vendorIdentifier);
            }

            if (productIdentifier) {
                ::CFNumberGetValue(CFNumberRef(productIdentifier),
                                   kCFNumberIntType,
                                   &value);

                info.d_ptr->productIdentifier = QString::number(value, 16);
                ::CFRelease(productIdentifier);
            }

            ports.append(info);
        }

        (void) ::IOObjectRelease(service);
    }

    (void) ::IOObjectRelease(iter);

    return ports;
}

QT_END_NAMESPACE_SERIALPORT
