---
title: Supported hardware
permalink: /docs/supported-hardware/
---

# Platforms

## Overview
A goal of OSS-7 is to support different hardware platforms and radio's.
The stack provides this portability by using portable C code for the higher layers, and providing a pluggable driver system for hardware specific implementations.
As explained in the [architecture section]({{ site.baseurl }}{% link _docs/architecture.md %}), OSS-7 has the concept of chips and platforms inside the hardware abstraction layer.
A chip implementation contains drivers for MCU peripherals or radio chips. A platform is a combination of a set of chips (for example an MCU and a radio chip), and describes the board wiring and features like LEDs or buttons. You can think of a platform as a specific board.

Currently we support the following platforms, in decreasing order of completeness/stability:

Platform        | MCU                                   | Radio                         | Toolchain         |
--------------- | ------------------------------------- | ----------------------------- | ----------------- |
B_L072Z_LRWAN1  | STMicroelectronics STM32L072CZ (Cortex-M0+) | Semtech SX1276 | gcc-arm-embedded |
EZR32LG_WSTK6200| Silicon Labs EZR32LG SoC (Cortex-M3)	| Silicon Labs si4460 			| gcc-arm-embedded  |
EFM32GG_STK3700 | Silicon Labs Giant Gecko (Cortex-M3)  | none on-board (Texas Instruments CC1101 extension board available)      | gcc-arm-embedded  |
EFM32HG_STK3400 | Silicon Labs Happy Gecko (Cortex-M0+) | none on-board (Texas Instruments CC1101 extension board available)      | gcc-arm-embedded  |


Currently the B_L072Z_LRWAN1 and EZR32LG_WSTK6200 boards are the only off the shelf, commercially available devkits which includes a radio. Because of this, these board is currently the easiest way to get started. The B_L072Z_LRWAN1 is probably the better option to get started. While the EZR32LG_WSTK6200 is a nice devkit it is not as cheap as we would want it to be. Also it appears to be out of stock lately and we think it might be discontinued soon.

Below you will find more information on each platform, and a section about supporting other platforms as well.


## B_L072Z_LRWAN1

The [B_L072Z_LRWAN1](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/b-l072z-lrwan1.html) is marketed by STMicroelectronics as an STM32L0 discovery kit for LoRa, but it can be used for DASH7 as well. It contains a [Murata CMWX1ZZABZ-091 module](http://wireless.murata.com/eng/products/rf-modules-1/lpwa/type-abz.html)
 which contains a STM32L072CZ MCU and a Semtech sx1276 RF chip together on a stand-alone module. This module is interesting because it allows to easily integrate a DASH7 modem on a custom design.

 ![B_L072Z_LRWAN1](https://i0.wp.com/blog.st.com/wp-content/uploads/RS7569_B_L072Z_side_antenna.jpg)

## EZR32LG_WSTK6200

The [EZR32LG_WSTK6200](https://www.silabs.com/products/wireless/wirelessmcu/Pages/ezr32lg-starter-kits.aspx) platform is a starter kit based on the EZR32 Leopard Gecko Wireless MCU. This SoC contains a Wonder Gecko Cortex-M3 combined with an RF chip (si4460).

![The EZR32LG devkit]({{site.baseurl}}/img/wstk6200.png)

To use the default UART using Breakout pad P4 (TX) and P6 (RX):
* PLATFORM_EZ32LG_WSTK6200A_CONSOLE_UART		0
* PLATFORM_EZ32LG_WSTK6200A_CONSOLE_LOCATION	1

Connect P4 to Yellow wire of FTDI connector
Connect P6 to Red wire of FTDI connector (if this wire is connected, make sure you keep the board powered)
Connect GND to Black wire of the FTDI connector
You can power the boad using the Red wire of the FTDI using the 5V (!) pin of the dev kit.

To use the VCOM (use ethernet port and telnet (port 4901)):
* PLATFORM_EZ32LG_WSTK6200A_CONSOLE_UART		3
* PLATFORM_EZ32LG_WSTK6200A_CONSOLE_LOCATION	1
* PLATFORM_USE_VCOM								ENABLE

initSensors() will initialize the Humidity and Temperature sensor on the devkit.
After initialization the values can be read using getHumidityAndTemperature()

## EFM32GG_STK3700
The [EFM32GG_STK3700](https://www.silabs.com/products/mcu/lowpower/Pages/efm32gg-stk3700.aspx) is a devkit for the SiLabs Giant Gecko MCU. An advantage is that a JLink programmer is already included on the board, making this is a cheap option (currently around 25 euro!).
A disadvantage of this platform is that you need to attach an external radio. We designed a CC1101-based module which can be plugged in the expansion port of the devkit, see below for the schematics.

## EFM32HG_STK3400
The [EFM32HG_STK3400](https://www.silabs.com/products/mcu/32-bit/Pages/efm32hg-stk3400.aspx) is very similar to the STK3700 but instead has a Cortex-M0+ instead of Cortex-M3 and a more capable LCD screen. The same CC1101 module as used for the STK3700 can be plugged into the expansion header.

## CC1101 RF module for Gecko devkits

The RF module is a plug-in module for the Gecko starter kits and is designed by University of Antwerp.
The RF module is based on the Texas Instruments (TI) CC1101 radio.
Currently there are two baluns available for this module, 433MHz and 868MHz.

![Gaint Gecko with CC1101 RF module]({{site.baseurl}}/img/GG_CC1101.jpg)

Schematics and Eagle files are available in the [git repository](https://github.com/mosaic-lopow/dash7-ap-open-source-stack/tree/master/hardware/stk3700-cc1101). If there are any questions, contact us through the mailing list.

## Other platforms

The list above only shows the readily available, public, platforms. It is of course possible to define your own platform for custom designs. Especially if you are using an MCU and radio which is already implemented it takes very little effort to add support to OSS-7 for your platform. Designing your own platform gives you the most flexibility regarding form factor and sensors specific for you use case of course. We are glad to assist in designing a custom board.
It is important to know that there are a number of parties who are currently in the process of designing devkits which will be commercially available,
so the choice should increase in the near future.

For more info on adding support for custom platforms or porting to new MCU's or RF chips please refer to the [porting section]({{ site.baseurl }}{% link _docs/porting.md %})

## Note on CC430 based platforms

At the moment we are not focusing on the CC430 based platform, mainly because ARM Cortex based platforms gives us more flexibility with regard to code size.
Also, in my experience, the free toolchains available for CC430 are not as robust as gcc-arm-embedded. TI's msp430-gcc (which supersedes mspgcc),
is still a young effort and there are some known problems with code size optimization. While OSS-7 has a HAL implementation to support the Wizzilab platform, this platform is not tested by us anymore since we stopped using cc430 based hardware for our projects. Furthermore, we are aware of crashes when running the current OSS-7 stack on cc430. Only the PHY layer is validated to work on cc430 for now. We currently lack time to look into this and would like to focus our efforts, but we would of course welcome patches from people who would still like to use cc430. The code for this platform will be removed in the near future, unless someone volunteers to maintain this. This means that using cc430 based platforms is strongly discouraged and not supported unless you want maintain/fix this.
