---
title: Platform notes B_L072Z_LRWAN1
permalink: /docs/platform-lrwan1/
---

# Programmer support

The board has an ST-LINK/V2-1 embedded to flash or debug the board. The ST-LINK debugger is currently not integrated in the OSS-7 build system so you can flash using `make flash-<appname>`. If you want to use this it is advised to convert the ST-LINK into a J-Link by using [firmware supplied by SEGGER](https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/). Besides being able to use the make target to flash,
this also allows logging using RTT (see [here for more info]({{ site.baseurl }}{% link _docs/logging.md %})).
