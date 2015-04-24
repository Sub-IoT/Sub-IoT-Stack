# Getting Started

[TOC]

## Hardware

For an overview of supported hardware, [look here](hardware.md)

## Prerequisites

- OSS-7 code. The code is hosted on [github](https://github.com/CoSys-Lab/dash7-ap-open-source-stack/commits/master), so either fork or clone the repository.
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

    $ make
    Scanning dependencies of target PLATFORM
    [  2%] Building C object framework/hal/platforms/EFM32GG_STK3700/CMakeFiles/PLATFORM.dir/stk3700_main.c.obj
    [  5%] Building C object framework/hal/platforms/EFM32GG_STK3700/CMakeFiles/PLATFORM.dir/stk3700_leds.c.obj
    [  8%] Building C object framework/hal/platforms/EFM32GG_STK3700/CMakeFiles/PLATFORM.dir/stk3700_userbutton.c.obj
    [ 11%] Building C object framework/hal/platforms/EFM32GG_STK3700/CMakeFiles/PLATFORM.dir/stk3700_debug.c.obj
    [ 13%] Building C object framework/hal/platforms/EFM32GG_STK3700/CMakeFiles/PLATFORM.dir/libc_overrides.c.obj
    [ 13%] Built target PLATFORM
    Scanning dependencies of target CHIP_CC1101
    [ 16%] Building C object framework/hal/chips/cc1101/CMakeFiles/CHIP_CC1101.dir/cc1101.c.obj
    [ 19%] Building C object framework/hal/chips/cc1101/CMakeFiles/CHIP_CC1101.dir/cc1101_interface.c.obj
    [ 22%] Building C object framework/hal/chips/cc1101/CMakeFiles/CHIP_CC1101.dir/cc1101_interface_spi.c.obj
    [ 22%] Built target CHIP_CC1101
    Scanning dependencies of target CHIP_EFM32GG
    [ 25%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/CMSIS/device/src/system_efm32gg.c.obj
    [ 27%] Building ASM object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/CMSIS/device/src/startup_gcc_efm32gg.s.obj
    [ 30%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/emlib/src/em_assert.c.obj
    [ 33%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/emlib/src/em_system.c.obj
    [ 36%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/emlib/src/em_cmu.c.obj
    [ 38%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/emlib/src/em_emu.c.obj
    [ 41%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/emlib/src/em_gpio.c.obj
    [ 44%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/emlib/src/em_usart.c.obj
    [ 47%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/emlib/src/em_rtc.c.obj
    [ 50%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/emlib/src/em_dma.c.obj
    [ 52%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/emlib/src/em_int.c.obj
    [ 55%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/kits/common/drivers/dmactrl.c.obj
    [ 58%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/kits/common/drivers/gpiointerrupt.c.obj
    [ 61%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/efm32gg_mcu.c.obj
    [ 63%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/efm32gg_uart.c.obj
    [ 66%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/efm32gg_spi.c.obj
    [ 69%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/efm32gg_atomic.c.obj
    [ 72%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/efm32gg_timer.c.obj
    [ 75%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/efm32gg_system.c.obj
    [ 77%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/efm32gg_gpio.c.obj
    [ 80%] Building C object framework/hal/chips/efm32gg/CMakeFiles/CHIP_EFM32GG.dir/efm32gg_pins.c.obj
    [ 80%] Built target CHIP_EFM32GG
    Scanning dependencies of target FRAMEWORK_random
    [ 83%] Building C object framework/components/random/CMakeFiles/FRAMEWORK_random.dir/random.c.obj
    [ 83%] Built target FRAMEWORK_random
    Scanning dependencies of target FRAMEWORK_node_globals
    [ 86%] Building C object framework/components/node_globals/CMakeFiles/FRAMEWORK_node_globals.dir/ng.c.obj
    [ 86%] Built target FRAMEWORK_node_globals
    Scanning dependencies of target FRAMEWORK_log
    [ 88%] Building C object framework/components/log/CMakeFiles/FRAMEWORK_log.dir/log.c.obj
    [ 88%] Built target FRAMEWORK_log
    Scanning dependencies of target FRAMEWORK_scheduler
    [ 91%] Building C object framework/components/scheduler/CMakeFiles/FRAMEWORK_scheduler.dir/scheduler.c.obj
    [ 91%] Built target FRAMEWORK_scheduler
    Scanning dependencies of target FRAMEWORK_timer
    [ 94%] Building C object framework/components/timer/CMakeFiles/FRAMEWORK_timer.dir/timer.c.obj
    [ 94%] Built target FRAMEWORK_timer
    Scanning dependencies of target framework
    [ 97%] Building C object framework/CMakeFiles/framework.dir/framework_bootstrap.c.obj
    Linking C static library libframework.a
    [ 97%] Built target framework
    Scanning dependencies of target phy_test
    [100%] Building C object apps/phy_test/CMakeFiles/phy_test.dir/phy_test.c.obj
    Linking C executable phy_test
    [100%] Built target phy_test

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
