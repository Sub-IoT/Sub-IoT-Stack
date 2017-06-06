---
title: Logging
permalink: /docs/logging/
---

# RTT Logging

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
