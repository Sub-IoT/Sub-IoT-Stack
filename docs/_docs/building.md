---
title: Building
permalink: /docs/building/
---


# Get prerequisites

- OSS-7 code. The code is hosted on [github](https://github.com/mosaic-lopow/dash7-ap-open-source-stack/commits/master), so either fork or clone the repository.
- [CMake](http://www.cmake.org/) (v2.8.12 or greater) as a flexible build system
- a GCC-based toolchain matching the target platform, for example [GCC ARM Embedded](https://launchpad.net/gcc-arm-embedded) for ARM Cortex-M based platforms. By default the build system assumes the GCC ARM Embedded toolchain is located in the PATH environment variable (meaning you can run `arm-none-eabi-gcc` without specifying the full path).
- [JLinkExe](https://www.segger.com/downloads/jlink) if you are using a JLink probe to flash/debug your target

# Run cmake

We will create a build directory and run cmake to generate the buildscript:

	$ mkdir build
	$ cd build
	$ cmake ../dash7-ap-open-source-stack//stack/ -DAPP_GATEWAY=y -DAPP_SENSOR_PUSH=y
	-- Cross-compiling using gcc-arm-embedded toolchain
	-- Cross-compiling using gcc-arm-embedded toolchain
	-- The C compiler identification is GNU 4.9.3
	-- The CXX compiler identification is GNU 4.9.3
	-- Detecting C compiler ABI info
	-- Detecting C compiler ABI info - failed
	-- Detecting C compile features
	-- Detecting C compile features - failed
	-- Detecting CXX compiler ABI info
	-- Detecting CXX compiler ABI info - failed
	-- Detecting CXX compile features
	-- Detecting CXX compile features - failed
	-- detected supported platforms: B_L072Z_LRWAN1 EFM32GG_STK3700 EFM32HG_STK3400 EZR32LG_Octa EZR32LG_USB01 EZR32LG_WSTK6200A EZRPi NUCLEO_STM32L152 OCTA_Gateway stm32f4_discovery
	-- selected platform: B_L072Z_LRWAN1
	-- The ASM compiler identification is GNU
	-- Found assembler: /home/glenn/bin/toolchains/gcc-arm-none-eabi-4_9-2014q4/bin/arm-none-eabi-gcc
	-- Added chip stm32l0xx
	-- Added chip sx127x
	-- Configuring done
	-- Generating done
	-- Build files have been written to: /home/glenn/projects/build


A quick run-down of what happens:
* We point cmake to the open source stack implementation: `../dash7-ap-open-source-stack/stack/` in this case
* We are using the default gcc-arm-embedded toolchain, which is in our path. If you prefer not to add the toolchain directory to the PATH you have to specify the path by passing this to cmake as in this example: `-DTOOLCHAIN_DIR=../gcc-arm-none-eabi-4_9-2015q3/`. If you would want to use another toolchain you have to pass the `-DCMAKE_TOOLCHAIN_FILE=...` option to point to the cmake configuration for the cross compiler.
* Based on the toolchain a number of supported platform options are available. By default the `B_L072Z_LRWAN1` platform is selected. If you want another platform you can specify this using `-DPLATFORM=<platform-name>`. Each subdirectory beneath `stack/framework/hal/platforms` contains a different platform, and the platform name to use is equal to the name of the subdirectory.
* A platform is a combination of one or more chips (MCU or RF) and the wiring between them. Based on the platform a number of chips will be added to the build, in this example the `stm32l0xx` MCU and the `sx127x` RF chip.
* Applications can be added by setting -DAPP_<name>=y. The name of the application is the name of a subdirectory of stack/apps, but uppercased. In this example we enabled the sensor and gateway application.

# Build it!

If your toolchain is setup correctly you should be able to build the stack and the configured application(s) now. An example of (shortened) output is below:

	$ make
	[ 18%] Built target d7ap
	[ 20%] Built target CHIP_SX127X
	[ 58%] Built target CHIP_STM32L0XX
	[ 65%] Built target PLATFORM
	[ 66%] Built target FRAMEWORK_COMPONENT_timer
	[ 70%] Built target FRAMEWORK_COMPONENT_aes
	[ 71%] Built target FRAMEWORK_COMPONENT_cli
	[ 73%] Built target FRAMEWORK_COMPONENT_compress
	[ 75%] Built target FRAMEWORK_COMPONENT_console
	[ 76%] Built target FRAMEWORK_COMPONENT_crc
	[ 78%] Built target FRAMEWORK_COMPONENT_fec
	[ 80%] Built target FRAMEWORK_COMPONENT_fifo
	[ 81%] Built target FRAMEWORK_COMPONENT_log
	[ 83%] Built target FRAMEWORK_COMPONENT_node_globals
	[ 85%] Built target FRAMEWORK_COMPONENT_pn9
	[ 86%] Built target FRAMEWORK_COMPONENT_random
	[ 88%] Built target FRAMEWORK_COMPONENT_scheduler
	[ 90%] Built target FRAMEWORK_COMPONENT_segger_rtt
	[ 91%] Built target FRAMEWORK_COMPONENT_shell
	[ 95%] Built target framework
	[100%] Built target sensor_push.elf

Note that this builds the chip drivers, the platform, the framework and the DASH7 stack implementation as a static library and links the applications.

# Build options

More build options to for example tweak parameters of the stack or platform specific settings can be configured through cmake. This can be done by passing -D options on the commandline (as above) or using the ccmake interactive console interface or the cmake-gui interface as shown below. The current values of all options can be displayed by executing `cmake -L`.

# Flashing

cmake will generate targets for flashing each application using JLink, by running `make flash-<appname>`. For example:

	$ make flash-sensor_push
	[100%] Built target sensor_push.elf
	Scanning dependencies of target flash-sensor_push
	SEGGER J-Link Commander V6.18a (Compiled Aug 11 2017 17:53:58)
	DLL version V6.18a, compiled Aug 11 2017 17:53:53


	Script file read successfully.
	Processing script file...

	J-Link connection not established yet but required for command.
	Connecting to J-Link via USB...O.K.
	Firmware: J-Link V9 compiled Jul 24 2017 17:37:57
	Hardware version: V9.40
	S/N: 269401926
	License(s): FlashBP, GDB
	OEM: SEGGER-EDU
	VTref = 3.311V

	Selecting SWD as current target interface.

	Selecting 10000 kHz as target interface speed

	Target connection not established yet but required for command.
	Device "STM32L072CZ" selected.


	Connecting to target via SWD
	Found SW-DP with ID 0x0BC11477
	Found SW-DP with ID 0x0BC11477
	Scanning AP map to find all available APs
	AP[1]: Stopped AP scan as end of AP map has been reached
	AP[0]: AHB-AP (IDR: 0x04770031)
	Iterating through AP map to find AHB-AP to use
	AP[0]: Core found
	AP[0]: AHB-AP ROM base: 0xF0000000
	CPUID register: 0x410CC601. Implementer code: 0x41 (ARM)
	Found Cortex-M0 r0p1, Little endian.
	FPUnit: 4 code (BP) slots and 0 literal slots
	CoreSight components:
	ROMTbl[0] @ F0000000
	ROMTbl[0][0]: E00FF000, CID: B105100D, PID: 000BB4C0 ROM Table
	ROMTbl[1] @ E00FF000
	ROMTbl[1][0]: E000E000, CID: B105E00D, PID: 000BB008 SCS
	ROMTbl[1][1]: E0001000, CID: B105E00D, PID: 000BB00A DWT
	ROMTbl[1][2]: E0002000, CID: B105E00D, PID: 000BB00B FPB
	Reset: Halt core after reset via DEMCR.VC_CORERESET.
	Reset: Reset device via AIRCR.SYSRESETREQ.
	Cortex-M0 identified.
	PC = 0800A9D4, CycleCnt = 00000000
	R0 = FFFFFFFF, R1 = FFFFFFFF, R2 = FFFFFFFF, R3 = FFFFFFFF
	R4 = FFFFFFFF, R5 = FFFFFFFF, R6 = FFFFFFFF, R7 = FFFFFFFF
	R8 = FFFFFFFF, R9 = FFFFFFFF, R10= FFFFFFFF, R11= FFFFFFFF
	R12= FFFFFFFF
	SP(R13)= 20005000, MSP= 20005000, PSP= FFFFFFFC, R14(LR) = FFFFFFFF
	XPSR = F1000000: APSR = NZCVq, EPSR = 01000000, IPSR = 000 (NoException)
	CFBP = 00000000, CONTROL = 00, FAULTMASK = 00, BASEPRI = 00, PRIMASK = 00

	Downloading file [sensor_push.bin]...
	Comparing flash   [100%] Done.
	Erasing flash     [100%] Done.
	Programming flash [100%] Done.
	Verifying flash   [100%] Done.
	J-Link: Flash download: Bank 0 @ 0x08000000: 4 ranges affected (96256 bytes)
	J-Link: Flash download: Total time needed: 8.306s (Prepare: 0.030s, Compare: 0.300s, Erase: 2.551s, Program: 5.370s, Verify: 0.050s, Restore: 0.002s)
	O.K.

	Loading binary file sensor_push.bin
	Reading 105196 bytes data from target memory @ 0x00000000.
	Verify successful.

	Reset delay: 0 ms
	Reset type NORMAL: Resets core & peripherals via SYSRESETREQ & VECTRESET bit.
	Reset: Halt core after reset via DEMCR.VC_CORERESET.
	Reset: Reset device via AIRCR.SYSRESETREQ.


	Script processing completed.

	[100%] Built target flash-sensor_push

If all went well the application should be running on the target.

# Tools

Make sure to checkout [pyd7a](https://github.com/MOSAIC-LoPoW/pyd7a) as well. This python package provides a collection of python modules, supporting the DASH7 Alliance Protocol in general, and OSS-7 in particular. It has an API to programmatically access an oss7 modem, tools to parse incoming packets and a webgui to interact with a modem and the network behind it.

# IDE Support

The OSS-7 buildsystem and code does not require a specific IDE. The user can choose which IDE to use, if any.
CMake supports generating project files for Eclipse CDT, CodeBlocks etc, instead of plain makefiles using the -G option.
Alternatively you can use an IDE which natively uses cmake projects like Qt Creator or JetBrain's CLion.

# Debugging

For debugging most toolchains come with a GDB client which you can attach to a GDB server which is compatible with your programmer.
For instance ARM Cortex-M platforms can use Segger's JLink programmers which comes with JLinkGDBServer. The arm-gcc-embedded toolchain
can then be used to connect with the JLinkGDBServer for flashing and debugging. Segger also provides the [Ozone](https://www.segger.com/ozone.html) GUI tool for debugging.

For Eclipse user: [here](http://gnuarmeclipse.livius.net/) you can find an Eclipse plugin to integrate JLink debugging in Eclipse.
Similarly, [EmbSysRegView](http://embsysregview.sourceforge.net/) is an Eclipse addin to view the registers of different MCU's.

We don't go into more detail about this here since this depends a lot on your favorite tools and the platform you are using.

# MS Windows support

While the above is written for Unix OS's (GNU/Linux and Mac OS X) it works on MS Windows as well. On MS Windows you should install the mingw32 compiler and use the "MinGW32 Makefiles" generator option when running cmake. Finally, you should use the `mingw32-make` command instead of `make`.
