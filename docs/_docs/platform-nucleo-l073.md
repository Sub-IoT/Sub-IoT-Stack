---
title: Platform notes NUCLEO_L073RZ
permalink: /docs/platform-nucleo-l073/
---

# Programmer support

The board has an ST-LINK/V2-1 embedded to flash or debug the board. The ST-LINK debugger is currently not integrated in the Sub-IoT build system.
You can use the ST-LINK for flashing by copying the binary to the mass storage device that is created when you attach the board to your PC using a USB cable.

If you want to be able to flash using `make flash-<appname>` it is advised to convert the ST-LINK into a J-Link by using [firmware supplied by SEGGER](https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/). Besides being able to use the make target to flash,
this also allows logging using RTT (see [here for more info]({{ site.baseurl }}{% link _docs/logging.md %})), and also exposes a VCOM port to the system (as does ST-LINK), see below.

# Console location

The serial console on this platform is configured to use UART2 by default. This has the advantage that is usable from the VCOM which is exposed by ST-LINK and JLink (see above).
This means that after plugging in your device using the ST-LINK USB connection you will get a serial interface (next to the USB mass storage device).
This interface can be used for interacting with the console.
Note that, on some Linux system, a service called ModemManager will claim the device upon detection. It will give up after a while, but it will result in communication problems,
each time you plug in. For a workaround look [here](https://linux-tips.com/t/prevent-modem-manager-to-capture-usb-serial-devices/284).
