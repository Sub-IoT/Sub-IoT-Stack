---
title: Platform notes B_L072Z_LRWAN1
permalink: /docs/platform-lrwan1/
---

# Programmer support

The board has an ST-LINK/V2-1 embedded to flash or debug the board. The ST-LINK debugger is currently not integrated in the OSS-7 build system.
You can use the ST-LINK for flashing by copying the binary to the mass storage device that is created when you attach the board to your PC using an USB cable.

If you want to be able to flash using `make flash-<appname>` it is advised to convert the ST-LINK into a J-Link by using [firmware supplied by SEGGER](https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/). Besides being able to use the make target to flash,
this also allows logging using RTT (see [here for more info]({{ site.baseurl }}{% link _docs/logging.md %})).
