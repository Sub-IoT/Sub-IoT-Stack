# Getting Started

[TOC]

## Hardware

For an overview of supported hardware, [look here](hardware.md)

## Building

### Get prerequisites

- OSS-7 code. The code is hosted on [github](https://github.com/mosaic-lopow/dash7-ap-open-source-stack/commits/master), so either fork or clone the repository.
- [CMake](http://www.cmake.org/) (v2.8.12 or greater) as a flexible build system
- a GCC-based toolchain matching the target platform for example [GCC ARM Embedded](https://launchpad.net/gcc-arm-embedded) for ARM Cortex-M bases platforms or [Texas Instruments MSP430-GCC](http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSPGCC/latest/index_FDS.html)
for CC430 based platforms

### Run cmake

We will create a build directory and run cmake to generate the buildscript for an ARM EFM32GG based platform in this case:

	$ mkdir build
	$ cd build
	$ cmake ../dash7-ap-open-source-stack/stack/ \
	        -DTOOLCHAIN_DIR=../gcc-arm-none-eabi-4_9-2015q3/ \
	        -DCMAKE_TOOLCHAIN_FILE=../dash7-ap-open-source-stack/stack/cmake/toolchains/gcc-arm-embedded.cmake \
	        -DPLATFORM=EFM32GG_STK3700 \
	        -DPLATFORM_EFM32GG_STK3700_RADIO=cc1101 \
	        -DAPP_D7AP_TEST=on
	-- Cross-compiling using gcc-arm-embedded toolchain
	-- Cross-compiling using gcc-arm-embedded toolchain
	-- detected supported platforms: EFM32GG_STK3700EFM32HG_STK3400ifestmatrix_tp1089
	-- selected platform: EFM32GG_STK3700
	-- The ASM compiler identification is GNU
	-- Found assembler: /Users/xtof/Workspace/tmp/gcc-arm-none-eabi-4_9-2015q3/bin/arm-none-eabi-gcc
	-- Configuring done
	-- Generating done
	-- Build files have been written to: /Users/xtof/Workspace/tmp/build

A quick run-down of the different arguments:
* we point cmake to the open source stack implementation: `../dash7-ap-open-source-stack/stack/`
* `TOOLCHAIN_DIR` points to our gcc cross compiler
* `CMAKE_TOOLCHAIN_FILE` points to the cmake configuration for our gcc cross compiler. In this case we are using the ARM cross compiler. If you are on a MSP430 based platform you can use `msp430-gcc.cmake` in the same directory.
* `PLATFORM` is one of the supported platforms (see: `dash7-ap-open-source-stack/stack/framework/hal/platforms`). Note that the build system will enable platforms based on the selected toolchain so make sure you have select the right toolchain for your target platform.
* `PLATFORM_<insert chosen platform here in capitals>_RADIO` identifies the radio we want to use (see: also `dash7-ap-open-source-stack/stack/framework/hal/chips`).
* turn on compilation of chosen app(s): `APP_<insert chosen application here in capitals>` (see:  `dash7-ap-open-source-stack/stack/apps`).

For example configuring a buildscript for the cc430 based wizzimote platform can be done like this:

	$ cmake ../dash7-ap-open-source-stack/stack/ \
	        -DTOOLCHAIN_DIR=../msp430-gcc-3.5.0.0/ \
	        -DCMAKE_TOOLCHAIN_FILE=../dash7-ap-open-source-stack/stack/cmake/toolchains/msp430-gcc.cmake \
	        -DPLATFORM=wizzimote \
	        -DAPP_PHY_TEST=on

### Build it!

If your toolchain is setup correctly you should be able to build the stack and the configured application(s) now:

	$ make
	Scanning dependencies of target CHIP_CC1101
	[  1%] Building C object framework/hal/platforms/platform/framework/hal/chips/cc1101/CMakeFiles/CHIP_CC1101.dir/cc1101.c.obj
	[  2%] Building C object framework/hal/platforms/platform/framework/hal/chips/cc1101/CMakeFiles/CHIP_CC1101.dir/cc1101_interface.c.obj
	[  3%] Building C object framework/hal/platforms/platform/framework/hal/chips/cc1101/CMakeFiles/CHIP_CC1101.dir/cc1101_interface_spi.c.obj
	[  3%] Built target CHIP_CC1101
	Scanning dependencies of target CHIP_EFM32GG
	...
	[ 97%] Built target d7ap
	Scanning dependencies of target d7ap_test.elf
	[ 98%] Building C object apps/d7ap_test/CMakeFiles/d7ap_test.elf.dir/d7ap_test.c.obj
	[100%] Linking C executable d7ap_test.elf
	[100%] Built target d7ap_test.elf

### Build options

More build options to for example tweak parameters of the stack or platform specific settings can be configured through cmake. This can be done by passing -D options on the commandline (as above) or using the ccmake interactive console interface of the cmake-gui interface as shown below. 

## IDE Support

The OSS-7 buildsystem and code does not require a specific IDE. The user can choose which IDE to use, if any.
CMake supports generating project files for Eclipse CDT, CodeBlocks etc, instead of plain makefiles using the -G option.
Alternatively you can use an IDE which natively uses cmake projects like Qt Creator or JetBrain's CLion.

For debugging most toolchains come with a GDB client which you can attach to a GDB server which is compatible with your programmer.
For instance ARM Cortex-M platforms can use Segger's JLink programmers which comes with JLinkGDBServer. The arm-gcc-embedded toolchain
can then be used to connect with the JLinkGDBServer for flashing and debugging. Segger also provides the JLinkDebugger GUI tool for debugging.

For Eclipse user: [here](http://gnuarmeclipse.livius.net/) you can find an Eclipse plugin to integrate JLink debugging in Eclipse. 
Similarly, [EmbSysRegView](http://embsysregview.sourceforge.net/) is an Eclipse addin to view the registers of different MCU's.

We don't go into more detail about this here since this depends a lot on your favorite tools and the platform you are using.

## MS Windows support

While the above is written for Unix OS's (GNU/Linux and Mac OS X) it works on MS Windows as well. On MS Windows you should install the mingw32 compiler and use the "MinGW32 Makefiles" generator option when running cmake. Finally, you should use the `mingw32-make` command instead of `make`.