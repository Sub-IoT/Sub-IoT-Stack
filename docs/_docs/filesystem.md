---
title: Filesystem
permalink: /docs/filesystem/
---

# Intro

D7AP [specifies](http://www.dash7-alliance.org/specification/) a structured filesystem containing a set of obligatory system files and optional user files. The system files contain all the configuration and status of the stack itself.
The user files, which are not defined by the spec, can be used for example for your specific sensor data.
The application using the D7AP stack interacts with the filesystem of the local node and with the filesystem of remote nodes, transparently. A file in the D7AP filesystem has associated properties like storage class (e.g. volatile, permanent) and permissions.

# Implementation

The D7AP filesystem implementation in OSS-7 can be found in (`stack/modules/d7ap/d7ap_fs.c`). For storing the system files it depends on a `blockdevice_t` which is an abstraction used to read and write blocks of memory. At the moment there are 2 concrete implementations of the blockdevice API: `blockdevice_driver_stm32_eeprom` which uses the embedded EEPROM of the STM32L MCU, and `blockdevice_driver_ram` which uses a buffer in RAM (and hence is volatile).

Currently the blockdevice stores the headers and contents of the systemfiles only, the user files (if any) are stored in RAM for now. We are not using a real filesystem like LittleFS for now, and do not implement features like wear levelling.

The data contained in the filesystem is defined in C arrays in `stack/modules/d7ap/d7ap_fs_data.c`. This data ends up in RAM, unless the platform defines a `PLATFORM_FS_SYSTEMFILES_IN_SEPARATE_LINKER_SECTION` cmake variable. This way, the filesystem data will end up in a separate linker sections (`.d7ap_fs_systemfiles`) which can then be moved by modyfing the linker script. This method is used on stm32l based platforms to move the filesystem to the region of the embedded EEPROM (see for example in the linker script `stack/framework/hal/platforms/B_L072Z_LRWAN1/STM32L072XZ.ld`). When a sperate linker section is used the buildsystem will make sure to remove this section from the resulting `<appname>-app.hex` and add it to `<appname>-eeprom-fs.hex`, while `<appname>-full.hex` will contain everything. Different make targets will be created as well, for instance `make flash-modem` will flash the complete application + EEPROM section, while `make flash-modem-app` and `make flash-modem-eeprom-fs` allow you to flash only the application or the EEPROM respectively.

The platform should define the blockdevice to use by defining a `static blockdevice_t* d7_systemfiles_blockdevice` in it's platform.h (see for instance `stack/framework/hal/platforms/B_L072Z_LRWAN1/inc/platform.h`).

The data in the systemfiles contained in `d7ap_fs_data.c` is not hardcoded, instead it is generated.
For this we use [pyd7a](https://github.com/MOSAIC-LoPoW/pyd7a), which provides an API to generate the files, together with [cog](https://bitbucket.org/ned/cog). Cog is a code generation tool which allows to run the python code embedded in comments in `d7ap_fs_data.c` and generate output, which is used to fill the C arrays. If wanted, the filesystem data can be updated by running `PYTHONPATH="<path to pyd7a>" python2 -m cogapp -c -r stack/modules/d7ap/d7ap_fs_data.c` (after installing pyd7a).

# Future work

- Allow to store user files in non volatile memory
- Enable RAM caching of some files which change frequently, with periodic flushing to permanent memory
- Enable using real filesystem implementations underneath of d7ap_fs
- Make it possible to use a filesystem which is generated out of tree (instead of `d7ap_fs_data.c`)
