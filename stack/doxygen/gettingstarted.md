# Getting Started

[TOC]

## Hardware

For an overview of supported hardware, [look here](hardware.md)

## Building

### Get prerequisites

- OSS-7 code. The code is hosted on [github](https://github.com/mosaic-lopow/dash7-ap-open-source-stack/commits/master), so either fork or clone the repository.
- [CMake](http://www.cmake.org/) (v2.8.12 or greater) as a flexible build system
- a GCC-based toolchain matching the target platform, for example [GCC ARM Embedded](https://launchpad.net/gcc-arm-embedded) for ARM Cortex-M based platforms. By default the build system assumes the GCC ARM Embedded toolchain is located in the PATH environment variable (meaning you can run `arm-none-eabi-gcc` without specifying the full path).
- [JLinkExe](https://www.segger.com/downloads/jlink) if you are using a JLink probe to flash/debug your target

### Run cmake

We will create a build directory and run cmake to generate the buildscript:

	$ mkdir build
	$ cd build
	$ cmake ../dash7-ap-open-source-stack/stack/ -DAPP_GATEWAY=y -DAPP_SENSOR=y
	-- Cross-compiling using gcc-arm-embedded toolchain
	-- Cross-compiling using gcc-arm-embedded toolchain
	-- detected supported platforms: EFM32GG_STK3700 EFM32HG_STK3400 EZR32LG_Octa EZR32LG_USB01 EZR32LG_WSTK6200A OCTA_Gateway stm32f4_discovery
	-- selected platform: EZR32LG_WSTK6200A
	-- The ASM compiler identification is GNU
	-- Found assembler: /home/glenn/bin/toolchains/gcc-arm-none-eabi-4_9-2014q4/bin/arm-none-eabi-gcc
	-- Added chip ezr32lg
	-- Added chip si4460
	-- Configuring done
	-- Generating done
	-- Build files have been written to: /home/glenn/oss7/build

A quick run-down of what happens:
* We point cmake to the open source stack implementation: `../dash7-ap-open-source-stack/stack/` in this case
* We are using the default gcc-arm-embedded toolchain, which is in our path. If you prefer not to add the toolchain directory to the PATH you have to specify the path by passing this to cmake as in this example: `-DTOOLCHAIN_DIR=../gcc-arm-none-eabi-4_9-2015q3/`. If you would want to use another toolchain you have to pass the `-DCMAKE_TOOLCHAIN_FILE=...` option to point to the cmake configuration for the cross compiler.
* Based on the toolchain a number of supported platform options are available. By default the `EZR32LG_WSTK6200A` platform is selected. If you want another platform you can specify this using `-DPLATFORM=<platform-name>`
* A platform is a combination of one or more chips (MCU or RF) and the wiring between them. Based on the platform a number of chips will be added to the build, in this example the `ezr32lg` MCU and the `si4460` RF chip.
* Applications can be added by setting -DAPP_<name>=y. The name of the application is the name of a subdirectory of stack/apps, but uppercased. In this example we enabled the sensor and gateway application.

For example configuring a buildscript for the cc430 based wizzimote platform can be done like this:

	$ cmake ../dash7-ap-open-source-stack/stack/ \
	        -DTOOLCHAIN_DIR=../msp430-gcc-3.5.0.0/ \
	        -DCMAKE_TOOLCHAIN_FILE=../dash7-ap-open-source-stack/stack/cmake/toolchains/msp430-gcc.cmake \
	        -DPLATFORM=wizzimote

### Build it!

If your toolchain is setup correctly you should be able to build the stack and the configured application(s) now. An example of (shortened) output is below:

	$ make
	Scanning dependencies of target CHIP_SI4460
	[  1%] Building C object framework/hal/platforms/platform/chips/si4460/CMakeFiles/CHIP_SI4460.dir/si4460.c.obj
	[  2%] Building C object framework/hal/platforms/platform/chips/si4460/CMakeFiles/CHIP_SI4460.dir/si4460_interface.c.obj
	[  6%] Built target CHIP_SI4460
	Scanning dependencies of target CHIP_EZR32LG
	[  7%] Building C object framework/hal/platforms/platform/chips/ezr32lg/CMakeFiles/CHIP_EZR32LG.dir/ezr32lg_adc.c.obj
	<snip>
	[ 44%] Built target CHIP_EZR32LG
	Scanning dependencies of target PLATFORM
	<snip>
	[ 61%] Built target PLATFORM
	<snip>
	Scanning dependencies of target d7ap
	<snip>
	[ 88%] Linking C static library libd7ap.a
	[ 88%] Built target d7ap
	Scanning dependencies of target gateway.elf
	<snip>
	[ 90%] Linking C executable gateway.elf
	[ 91%] Built target gateway.elf
	<snip>
	Scanning dependencies of target sensor.elf
	[ 95%] Building C object apps/sensor/CMakeFiles/sensor.elf.dir/sensor.c.obj
	[ 96%] Building C object apps/sensor/CMakeFiles/sensor.elf.dir/version.c.obj
	[ 97%] Linking C executable sensor.elf
	[ 97%] Built target sensor.elf

Note that this builds the chip drivers, the platform and the DASH7 stack implementation as a static library and links the applications.

### Build options

More build options to for example tweak parameters of the stack or platform specific settings can be configured through cmake. This can be done by passing -D options on the commandline (as above) or using the ccmake interactive console interface of the cmake-gui interface as shown below. The current values of all options can be displayed by executing `cmake -L`.

## Flashing

cmake will generate targets for flashing each application using JLink, by running `make flash-<appname>`. For example:

	$ make flash-sensor
	<snip>
	[100%] Built target sensor.elf
	SEGGER J-Link Commander V6.10m (Compiled Nov 10 2016 18:38:45)
	DLL version V6.10m, compiled Nov 10 2016 18:38:36
	<snip>
	Connecting to J-Link via USB...O.K.
	Firmware: Silicon Labs J-Link Pro OB compiled Jan 22 2016 15:00:47
	Hardware version: V4.00
	S/N: 440058314
	IP-Addr: DHCP (no addr. received yet)
	VTref = 3.341V

	Selecting SWD as current target interface.
	Selecting 10000 kHz as target interface speed

	Target connection not established yet but required for command.
	Device "EZR32LG330F256RXX" selected.

	Found SWD-DP with ID 0x2BA01477
	AP-IDR: 0x24770011, Type: AHB-AP
	Found Cortex-M3 r2p1, Little endian.
	<snip>
	Downloading file [sensor.bin]...
	Comparing flash   [100%] Done.
	Verifying flash   [100%] Done.
	J-Link: Flash download: Flash download skipped. Flash contents already match
	O.K.

	Loading binary file sensor.bin
	Reading 94072 bytes data from target memory @ 0x00000000.
	Verify successful.
	<snip>
	[100%] Built target flash-sensor

If all went well the application should be running on the target.

## RTT Logging

If you are using a platform which uses JLink then logging will be enabled by default and redirect to the fast RTT interface. This interface makes use of JLink connection, meaning you do not need extra wires (UART or USB) for logging.
You can read the logs using `JLinkRTTClient`. This will wait for a JLink connection and print all RTT logs.
Opening a JLink connection can be done using `make jlink-open` in your build directory.
If you open `JLinkRTTClient` in one terminal and execute `make flash-sensor jlink-open` in another you will get this output:

	$ JLinkRTTClient
	###RTT Client: ************************************************************
	###RTT Client: *           SEGGER MICROCONTROLLER GmbH & Co KG            *
	###RTT Client: *   Solutions for real time microcontroller applications   *
	###RTT Client: ************************************************************
	###RTT Client: *                                                          *
	###RTT Client: *  (c) 2012 - 2016  SEGGER Microcontroller GmbH & Co KG    *
	###RTT Client: *                                                          *
	###RTT Client: *     www.segger.com     Support: support@segger.com       *
	###RTT Client: *                                                          *
	###RTT Client: ************************************************************
	###RTT Client: *                                                          *
	###RTT Client: * SEGGER J-Link RTT Client   Compiled Nov 10 2016 18:39:09 *
	###RTT Client: *                                                          *
	###RTT Client: ************************************************************

	###RTT Client: -----------------------------------------------
	###RTT Client: Connecting to J-Link RTT Server via localhost:19021 ..... Connected.
	SEGGER J-Link V6.10m - Real time terminal output
	Silicon Labs J-Link Pro OB compiled Jan 22 2016 15:00:47 V4.0, SN=440058314
	Process: JLinkExe
	###RTT Client: Connection closed by J-Link DLL. Going to reconnect.
	###RTT Client: Connecting to J-Link RTT Server via localhost:19021 . Connected.
	SEGGER J-Link V6.10m - Real time terminal output
	Silicon Labs J-Link Pro OB compiled Jan 22 2016 15:00:47 V4.0, SN=440058314
	Process: JLinkExe

	[000] Device booted

	[001] Int T: 31.8 C
	[002] Ext T: 27.7 C
	[003] Ext H: 30.5
	[004] Batt 3297 mV

Note that you can enable different log sources separately by using cmake settings ( x_LOG_ENABLED).

## IDE Support

The OSS-7 buildsystem and code does not require a specific IDE. The user can choose which IDE to use, if any.
CMake supports generating project files for Eclipse CDT, CodeBlocks etc, instead of plain makefiles using the -G option.
Alternatively you can use an IDE which natively uses cmake projects like Qt Creator or JetBrain's CLion.

## Debugging

For debugging most toolchains come with a GDB client which you can attach to a GDB server which is compatible with your programmer.
For instance ARM Cortex-M platforms can use Segger's JLink programmers which comes with JLinkGDBServer. The arm-gcc-embedded toolchain
can then be used to connect with the JLinkGDBServer for flashing and debugging. Segger also provides the [Ozone](https://www.segger.com/ozone.html) GUI tool for debugging.

For Eclipse user: [here](http://gnuarmeclipse.livius.net/) you can find an Eclipse plugin to integrate JLink debugging in Eclipse.
Similarly, [EmbSysRegView](http://embsysregview.sourceforge.net/) is an Eclipse addin to view the registers of different MCU's.

We don't go into more detail about this here since this depends a lot on your favorite tools and the platform you are using.

## MS Windows support

While the above is written for Unix OS's (GNU/Linux and Mac OS X) it works on MS Windows as well. On MS Windows you should install the mingw32 compiler and use the "MinGW32 Makefiles" generator option when running cmake. Finally, you should use the `mingw32-make` command instead of `make`.
