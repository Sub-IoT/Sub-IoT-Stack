---
title: Filesystem
permalink: /docs/filesystem/
---

# Intro

[D7AP] (https://www.dash7-alliance.org/download-specification/) specifies a structured filesystem containing a set of obligatory system files and optional user files. The system files contain all the configuration and status of the stack itself.
The user files, which are not defined by the spec, can be used for example for your specific sensor data.
The application using the D7AP stack interacts with the filesystem of the local node and with the filesystem of remote nodes, transparently. A file in the D7AP filesystem has associated properties like storage class (e.g. volatile, permanent) and permissions.

# Implementation

The D7AP filesystem implementation in Sub-IoT can be found in (`stack/modules/d7ap/d7ap_fs.c`). For storing the system files it depends on a `blockdevice_t` which is an abstraction used to read and write blocks of memory. At the moment there are 2 concrete implementations of the blockdevice API: `blockdevice_driver_stm32_eeprom` which uses the embedded EEPROM of the STM32L MCU, and `blockdevice_driver_ram` which uses a buffer in RAM (and hence is volatile).

Currently the blockdevice stores the headers and contents of the systemfiles only, the user files (if any) are stored in RAM for now. We are not using a real filesystem like LittleFS for now, and do not implement features like wear leveling.

The data contained in the filesystem is defined in C arrays in `stack/fs/d7ap_fs_data.c`. This data ends up in RAM, unless the platform defines a `PLATFORM_FS_SYSTEMFILES_IN_SEPARATE_LINKER_SECTION` cmake variable. This way, the filesystem data will end up in a separate linker sections (`.d7ap_fs_permanent_files_section`, `.d7ap_fs_metadata_section`) which can then be moved by modifying the linker script. This method is used on stm32l based platforms to move the filesystem to the region of the embedded EEPROM (see for example in the linker script `stack/framework/hal/platforms/B_L072Z_LRWAN1/STM32L072XZ.ld`). When a separate linker section is used the buildsystem will make sure to remove this section from the resulting `<appname>-app.hex` and add it to `<appname>-eeprom-fs.hex`, while `<appname>-full.hex` will contain everything. Different make targets will be created as well, for instance `make flash-modem` will flash the complete application + EEPROM section, while `make flash-modem-app` and `make flash-modem-eeprom-fs` allow you to flash only the application or the EEPROM respectively.

The platform should define the blockdevices to use for the filesystem. The filesystem requires 3 (mandatory) blockdevices: 1 for the filesystem metadata, 1 for permanent file data and 1 for volatile file data. Logically, the first 2 should use blockdevice driver making use of persistent memory if the platform allows. On STM32L they are stored on the embedded EEPROM.
To do this, the platform should define `PLATFORM_METADATA_BLOCKDEVICE`, `PLATFORM_PERMANENT_BLOCKDEVICE` and `PLATFORM_VOLATILE_BLOCKDEVICE` in its platform.h (see for instance `stack/framework/hal/platforms/B_L072Z_LRWAN1/inc/platform.h`).

The data in the systemfiles contained in `d7ap_fs_data.c` is not hardcoded, instead it is generated.
For this we use [pyd7a](https://github.com/Sub-IoT/pyd7a), which provides an API to generate the files, together with [cog](https://github.com/nedbat/cog). Cog is a code generation tool which allows to run the python code embedded in comments in `d7ap_fs_data.c` and generate output, which is used to fill the C arrays. If wanted, the filesystem data can be updated by running `PYTHONPATH="<path to pyd7a>" python3 -m cogapp -c -r stack/fs/d7ap_fs_data.c` (after installing pyd7a).

The value of the systemfiles in `d7ap_fs_data.c` is provided as a default. If you want to override it for your application you can do it out-of-tree to keep the source in sync with upstream. To do so, set the `MODULE_D7AP_FS_USE_DEFAULT_SYSTEMFILES` cmake variable to `false` and define `fs_systemfiles` and `fs_systemfiles_file_offsets` in your application code (which can be [out-of-tree]({{ site.baseurl }}{% link _docs/out-of-tree.md %})). The easiest way is to copy `d7ap_fs_data.c` to your application directory, strip everything besides the `fs_systemfiles` and `fs_systemfiles_file_offsets` definitions, adapt file contents where needed and add it to the `APP_BUILD` sources in the app's CMakeLists.txt.

# Future work

- Volatile files which are not generated in the FS data (but created at runtime) will lose their D7 file header on reboot, since this header is currently stored in the file and thus in this case in volatile memory. This header should also move to permanent storage to allow this to work. For now: either generate the volatile file during filesystem creation or re-add the file on each reboot.
- Enable RAM caching of some files which change frequently, with periodic flushing to permanent memory
- Enable using real filesystem implementations underneath of d7ap_fs
