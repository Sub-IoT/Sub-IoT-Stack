# Getting Started

[TOC]

## Hardware

For an overview of supported hardware, [look here](hardware.md)

## Prerequisites

- OSS-7 code. The code is hosted on [github](https://github.com/mosaic-lopow/dash7-ap-open-source-stack/commits/master), so either fork or clone the repository.
- [CMake](http://www.cmake.org/) (v2.8.12 or greater) as a flexible build system
- a GCC-based toolchain matching the target platform for example [GCC ARM Embedded](https://launchpad.net/gcc-arm-embedded) for ARM Cortex-M bases platforms or [Texas Instruments MSP430-GCC](http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSPGCC/latest/index_FDS.html)
for CC430 based platforms

## Configuring the build

OSS-7 uses [CMake](http://www.cmake.org/) to configure the buildproces settings and toolchain.
This can be done using the cmake command, using the ccmake commandline GUI tool, or using the cmake-gui tool.
We will use the cmake-gui tool for now.
Note: the following steps are executed on a Linux system, but should be applicable to MS Windows or Mac OS X as well.

### Configure source and build paths

After starting cmake-gui you first have to provide the path to the source directory and to the build directory.
The source directory should be set to <0SS7 clone dir>/stack .
The build directory can be anywhere you prefer, as long as it is outside of the source directory.

![Configuring the source and build paths](cmake-1.png)

### Configuring the toolchain

After configuring the paths as described above you need to push the "Configure" button.
The following dialog will popup:

![Configuring the toolchain](cmake-2.png)

Here you should select "Specify toolchain for cross-compiling" and click "Next".
In the next step you should provide the path to the toolchain definition file.
These files are located in <OSS7 clone dir>/stack/cmake/toolchains.
Select the correct toolchain file and press "Finish".
After finishing cmake will return an error, unless the toolchain directory happens to be in your PATH.
You can specify the toolchain directory by locating the TOOLCHAIN_DIR variable in the list of cmake variables,
and entering the correct path to the toolchain installation. In this case i entered the path to the gcc-arm-none-eabi-4_9-2014q4 
toolchain on my machine. If you press "Configure" again the error should be gone and you should see something like this:

![Configuring the toolchain path](cmake-3.png)

### Configure the OSS-7 build settings

After the toolchain configuration is completed we can continue with specifying the OSS7 settings.
First you should select the platform you are going to use. This is done with the PLATFORM variable,
where you can select the platform using the dropdown menu. This dropdown is automatically filled with all possible platforms,
supported by the selected toolchain. In this example i'm using the EFM32GG_STK3700 platform.
Next we should select the radio chip to use. This in turn depends on the selected platform. 
You can select the radio chip using the dropdown in the PLATFORM_<platform name>_RADIO variable (PLATFORM_EFM32GG_STK3700_RADIO in my case), 
we select the cc1101 radio driver here.

By now we have configured the basic setting to build the OSS7 framework. More tuning can be done by setting other variables like FRAMEWORK_LOG_ENABLED etc,
but we will keep these settings to default for now. Finally, we need to select the applications which we want to build.
All available applications in the parameter lists are present as boolean values named ass APP_<name>.
You should enable one or more applications and press "Configure" again. Finally, we should press "Generate" after which cmake generates the build script.

![Configuring the OSS-7 build settings](cmake-4.png)

## Building

After generating the build script in the previous step we can exit cmake-gui and build OSS-7 by issueing the make command in the build directory:

![Make](make.png)

## IDE Support

The OSS-7 buildsystem and code does not require a specific IDE. The user can choose which IDE to use, if any.
CMake supports generating project files for Eclipse CDT, CodeBlocks etc, instead of plain makefiles.
Alternatively you can use an IDE which natively uses cmake files like Qt Creator or JetBrain's CLion.

For debugging most toolchains come with a GDB client which you can attach to a GDB server which is compatible with your programmer.
For instance ARM Cortex-M platforms can use Segger's JLink programmers which come with JLinkGDBServer. The arm-gcc-embedded toolchain
can then be used to connect with the JLinkGDBServer for flashing and debugging. 

For Eclipse user: [here](http://gnuarmeclipse.livius.net/) you can find an Eclipse plugin to integrate JLink debugging in Eclipse. 
Similarly, [EmbSysRegView](http://embsysregview.sourceforge.net/) is an Eclipse addin to view the registers of different MCU's.

We don't go into more detail about this here since this dependents a lot on your favorite tools and the platform you are using.

## Logging

You can use the d7-oss-logger.py script in tools\PyLogger to log debug or application data.
