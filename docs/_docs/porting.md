---
title: Porting
permalink: /docs/porting/
---

If you want to run Sub-IoT on a board which is not listed in the [supported hardware section]({{ site.baseurl }}{% link _docs/hardware.md %}) there is some porting effort required. There are two main options:
- Custom board reusing existing MCU and radio drivers from a supported platform
- Custom board with an MCU and/or radio chip for which no driver exists yet

In the first case the porting effort is very low, it is only required to add a new platform in `stack/framework/hal/platforms`. The subdirectory name equals the name of the new platform. Inside this subdirectory there should be a `CMakeLists.txt`, a `platform.h` and a .c file implementing the `main()` entry point for your platform. The easiest way would be to copy the files from a resembling platform and adapt it to your needs. Basically what needs to be done at the platform level is the initialization of the platform, the wiring of the chips and platform specific configuration. Please note that the Sub-IoT buildsystem allows keeping custom platform (or chips) outside of the main source tree if you are not able to share this, see [out-of-tree section]({{ site.baseurl }}{% link _docs/out-of-tree.md %}) for more info.

In the second case the HAL API for the new MCU and/or the radio driver for the new radio chip needs to be implemented. While this takes some more effort it is certainly feasible, especially if you already have knowledge about the chip.

If a driver for a new MCU is required the easiest would be to start with this part, to get the basics like clock, timer, GPIO, SPI, ... working. Once this is done the radio driver can be ported as well if required.

More concrete steps to take:
- make sure you can compile for an existing platform (by following the [building section]({{ site.baseurl }}{% link _docs/building.md %})), to make sure your toolchain is working
- add a new platform (see above)
- add a new chip for the MCU in `stack/framework/hal/chips` containing the
implementation of the HAL API (refer to other chips and the headers)
- configure your platform to use this new chip by adding `ADD_CHIP("<chip name>")` in
the platform's `CMakeLists.txt`
- start with enabling only a simple application like `simple_leds` first

At this point the basic structure is there and you can start implementing the
HAL. If useful you can even connect an external RF chip (like the sx1276)
enabling you to test the HAL implementation and run the complete stack before
porting the RF driver itself.

Please let us know if we can help!
