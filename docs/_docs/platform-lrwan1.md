---
title: Platform notes B_L072Z_LRWAN1
permalink: /docs/platform-lrwan1/
---

# Programmer support

The board has an ST-LINK/V2-1 embedded to flash or debug the board. The ST-LINK debugger is currently not integrated in the Sub-IoT build system.
You can use the ST-LINK for flashing by copying the binary to the mass storage device that is created when you attach the board to your PC using a USB cable.

If you want to be able to flash using `make flash-<appname>` it is advised to convert the ST-LINK into a J-Link by using [firmware supplied by SEGGER](https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/). Besides being able to use the make target to flash,
this also allows logging using RTT (see [here for more info]({{ site.baseurl }}{% link _docs/logging.md %})), and also exposes a VCOM port to the system (as does ST-LINK), see below.

# Console location

The serial console on this platform is configured to use UART2 by default. This has the advantage that is usable from the VCOM which is exposed by ST-LINK and JLink (see above).
This means that after plugging in your device using the ST-LINK USB connection you will get a serial interface (next to the USB mass storage device). This interface can be used for interacting with the console or the serial modem interface.
More information on how to enable the JLink VCOM functionality can be found [here](https://wiki.segger.com/Using_J-Link_VCOM_functionality).

# Low power operation

To configure the board for low power operation:
- connect pin 1 and 2 of JP9, so TCXO is not powered continuously but can be powered through pin
- set cmake option PLATFORM_SX127X_USE_VCC_TXCO so the driver will take care of powering the TCXO
- Disconnect the ST-LINK  (note you will not be able to debug flash anymore) by removing SB28 and SB29 and SB37
- If you want to power through CN13 then you need to remove SB6 and R26 as well, so the voltage regulator is disabled
