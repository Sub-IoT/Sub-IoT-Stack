---
title: Platform notes EZR32LG_WSTK6200a
permalink: /docs/platform-wstk6200a/
---

To use the default UART using Breakout pad P4 (TX) and P6 (RX):
* PLATFORM_CONSOLE_UART		0
* PLATFORM_CONSOLE_LOCATION	1

Connect P4 to Yellow wire of FTDI connector
Connect P6 to Red wire of FTDI connector (if this wire is connected, make sure you keep the board powered)
Connect GND to Black wire of the FTDI connector
You can power the board using the Red wire of the FTDI using the 5V (!) pin of the devkit.

To use the VCOM (use ethernet port and telnet (port 4901)):
* PLATFORM_CONSOLE_UART		3
* PLATFORM_CONSOLE_LOCATION	1
* PLATFORM_USE_VCOM								ENABLE

initSensors() will initialize the Humidity and Temperature sensor on the devkit.
After initialization the values can be read using getHumidityAndTemperature()
