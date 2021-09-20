---
title: Logging
permalink: /docs/logging/
---

# RTT Logging

If you are using a platform which uses JLink then logging will be enabled by default and redirect to the fast RTT interface. This interface makes use of JLink connection, meaning you do not need extra wires (UART or USB) for logging.
You can read the logs using `JLinkRTTClient`. This will wait for a JLink connection and print all RTT logs.
Opening a JLink connection can be done using `make jlink-open` in your build directory. Make sure the firmware is built with `FRAMEWORK_LOG_ENABLED` and `FRAMEWORK_LOG_OUTPUT_ON_RTT` enabled.
If you open `JLinkRTTClient` in one terminal and execute `make flash-sensor_push jlink-open` in another this will flash the sensor_push example application and keep the JLink connection open. In the JLinkRTTClient window you will get output like this:

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

Note that you can enable different log sources separately by using cmake settings (x_LOG_ENABLED).

On some platforms (eg those based on stm32l0) it seems necessary to reset the node before output starts appearing. You can do this by entering `r` and `g` (reset and go) in the JLink console. 

# RTT logging with multiple targets attached

The `jlink-open` make target assumes there is only one JLink attached to the host system. If you want to open 2 (or more) JLink connections to view the logging of multiple devices, you will need to open a JLink connection manually.
This can be done as shown below:

	JLinkExe -SelectEmuBySN 518007358 -If SWD -Device EZR32LG330F256RXX -AutoConnect 1 -RTTTelnetPort 19021 -Speed 10000

Important to note here are the `-SelectEmuBySN <sn>` and `-RTTTelnetPort <port>` parameters. The first one allows you to specify which JLink adapter to use. The second parameter tells JLinkExe on which port it should listen for RTT clients. You can pass the same parameter to JLinkRTTClient:

	JLinkRTTClient -RTTTelnetPort 19021

This way multiple JLink adapters can be used at the same time, each which a different SN and port.
Note that flashing can be done using the JLinkExe prompt as well using the `loadfile` command.
Refer to the JLink documentation for more info

# UART logging

TODO
