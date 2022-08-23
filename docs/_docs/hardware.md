---
title: Supported hardware
permalink: /docs/supported-hardware/
---

# Platforms

## Overview
A goal of Sub-IoT is to support different hardware platforms and radio's.
The stack provides this portability by using portable C code for the higher layers, and providing a pluggable driver system for hardware specific implementations.
As explained in the [architecture section]({{ site.baseurl }}{% link _docs/architecture.md %}), Sub-IoT has the concept of chips and platforms inside the hardware abstraction layer.
A chip implementation contains drivers for MCU peripherals or radio chips. A platform is a combination of a set of chips (for example an MCU and a radio chip), and describes the board wiring and features like LEDs or buttons. You can think of a platform as a specific board.

Currently we support the following platforms, in decreasing order of completeness/stability:

Platform        | MCU                                   | Radio                         |
--------------- | ------------------------------------- | ----------------------------- |
B_L072Z_LRWAN1  | STMicroelectronics STM32L072CZ (Cortex-M0+) | Semtech SX1276 |
MURATA_ABZ  | STMicroelectronics STM32L072CZ (Cortex-M0+) | Semtech SX1276 |
NUCLEO_L073RZ   | STMicroelectronics STM32L073RZ (Cortex-M0+) | Semtech SX1276 (SX1276MB1MAS shield)|
EZR32LG_WSTK6200| Silicon Labs EZR32LG SoC (Cortex-M3)	| Silicon Labs si4460 			|
EFM32GG_STK3700 | Silicon Labs Giant Gecko (Cortex-M3)  | none on-board (Texas Instruments CC1101 extension board available)      |

Currently the B_L072Z_LRWAN1, NUCLEO_L073RZ (+SX1276MB1MAS) and EZR32LG_WSTK6200 boards are the only off the shelf, commercially available devkits which includes a radio. Because of this, these boards are the easiest way to get started. The B_L072Z_LRWAN1 or the NUCLEO_L073RZ are currently the best options. While the EZR32LG_WSTK6200 is a nice devkit it is not as cheap as we would want it to be. Also it appears to be out of stock lately, and we think it might be discontinued soon. Additionally, the sx1276 driver is more feature complete and better tested than the si4460 driver, at the moment.

Below you will find more information on each platform, and a section about supporting other platforms as well.


## B_L072Z_LRWAN1

The [B_L072Z_LRWAN1](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/b-l072z-lrwan1.html) is marketed by STMicroelectronics as an STM32L0 discovery kit for LoRa, but it can be used for DASH7 as well. It contains a [Murata CMWX1ZZABZ-091 module](http://wireless.murata.com/eng/products/rf-modules-1/lpwa/type-abz.html)
 which contains a STM32L072CZ MCU and a Semtech sx1276 RF chip together on a stand-alone module (the Murata Type ABZ).
 Note that this platform (and all platforms using the Murata module) only supports the 868 and 915 MHz bands, not the 433 MHz band. See the [platform notes]({{ site.baseurl }}{% link _docs/platform-lrwan1.md %}) for more specific information on how to use this platform.

 ![B_L072Z_LRWAN1](https://i0.wp.com/blog.st.com/wp-content/uploads/RS7569_B_L072Z_side_antenna.jpg)

## MURATA_ABZ

The [Murata Type ABZ](https://wireless.murata.com/eng/products/rf-modules-1/lpwa/type-abz.html)
 which contains a STM32L072CZ MCU and a Semtech sx1276 RF chip together on a stand-alone module. This platform is very similar to B_L072Z_LRWAN1 except that it only contains what is needed for Murata based designs.
 Compared to B_L072Z_LRWAN1 this platform does not include LED and button definitions.
This module is interesting because it allows to easily integrate a DASH7 modem on a custom design. Note that this module only supports the 868 and 915 MHz bands, not the 433 MHz band.


## NUCLEO-L073RZ

The [NUCLEO-L073RZ](http://www.st.com/en/evaluation-tools/nucleo-l073rz.html) is a Nucleo-64 type of development board from STMicroelectronics. The MCU is basically the same as the B_L072Z_LRWAN1 above, so it is using the same HAL driver. The board does not include a radio, but a separate [SX1276MB1xAS](https://os.mbed.com/components/SX1276MB1xAS/) containing a Semtech sx1276 can be ordered. Here as well, the radio is reusing the same driver as with the Murata module. The platform is very comparable to the B_L072Z_LRWAN1 overall. A distinction is that the SX1276MB1xAS has a LF output, which allows to use the 433 MHz band, while the Murata module only supports the 868 and 915 MHz bands. See the [platform notes]({{ site.baseurl }}{% link _docs/platform-nucleo-l073.md %}) for more specific information on how to use this platform.

![The nucleo-l073rz + SX1276MB1xAS devkit]({{site.baseurl}}/img/nucleo.jpg)

## EZR32LG WSTK6200

The EZR32LG WSTK6200 platform is a starter kit based on the EZR32 Leopard Gecko Wireless MCU. This SoC contains a Wonder Gecko Cortex-M3 combined with an RF chip (si4460).
See the [platform notes]({{ site.baseurl }}{% link _docs/platform-wstk6200a.md %}) for more specific information on how to use this platform.

![The EZR32LG devkit]({{site.baseurl}}/img/wstk6200.png)



## EFM32GG STK3700
The [EFM32GG STK3700](https://www.silabs.com/development-tools/mcu/32-bit/efm32gg-starter-kit) is a devkit for the SiLabs Giant Gecko MCU. An advantage is that a JLink programmer is already included on the board, making this is a cheap option (currently around 25 euro!).
A disadvantage of this platform is that you need to attach an external radio. We designed a CC1101-based module which can be plugged in the expansion port of the devkit, see below for the schematics.


## CC1101 RF module for Gecko devkits

The RF module is a plug-in module for the Gecko starter kits and is designed by University of Antwerp.
The RF module is based on the Texas Instruments (TI) CC1101 radio.
Currently there are two baluns available for this module, 433MHz and 868MHz.

![Gaint Gecko with CC1101 RF module]({{site.baseurl}}/img/GG_CC1101.jpg)

Schematics and Eagle files are available in the [git repository](https://github.com/Sub-IoT/Sub-IoT-Stack/tree/master/hardware/stk3700-cc1101). If there are any questions, contact us through the mailing list.

## Other platforms

The list above only shows the readily available, public, platforms. It is of course possible to define your own platform for custom designs. Especially if you are using an MCU and radio which is already implemented, it takes very little effort to add support to Sub-IoT for your platform. Designing your own platform gives you the most flexibility regarding form factor and sensors specific for your use case. We are glad to assist in designing a custom board.
It is important to know that there are a number of parties who are currently in the process of designing devkits which will be commercially available,
so the choice should increase in the near future.

For more info on adding support for custom platforms or porting to new MCU's or RF chips please refer to the [porting section]({{ site.baseurl }}{% link _docs/porting.md %}).
