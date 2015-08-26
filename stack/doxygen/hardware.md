# Hardware

A goal of OSS-7 is to support different hardware platforms and radio's. 
The stack provides this portability by using portable C code for the higher layers, and providing a pluggable driver system for hardware specific implementations. 
As explained in the [architecture documentation](architecture.md) OSS-7 has the concept of chips and platforms inside the hardware abstraction layer.
A chip implementation contains drivers for MCU peripherals or radio chips. A platform is a combination of an MCU and a radio chips and describes the board wiring and features like LEDs or buttons. 

Currently we support the following plaforms, in decreasing order of completeness/stability:

Platform        | MCU                                   | Radio                         | Toolchain         | 
--------------- | ------------------------------------- | ----------------------------- | ----------------- | 
EFM32GG_STK3700 | EnergyMicro Giant Gecko (Cortex-M3)   | Texas Instruments CC1101      | gcc-arm-embedded  |
wizzimote       | Texas Instruments CC430 (MSP430)      | Texas Instruments CC1101 (SoC)| msp430-gcc        |

The [EFM32GG_STK3700](https://www.silabs.com/products/mcu/lowpower/Pages/efm32gg-stk3700.aspx) is currently the most used by us, and thus the best supported.
A disadvantage of this platform is that you need to attach an external CC1101. We designed a CC1101-based module which can be plugged in the expansion port of the devkit, see below for the schematics.
Next to this, we are working on a PCB design for a devkit containing Giant Gecko and CC1101 as well, which we will opensource shortly.

At the moment we are not focusing on the CC430 based platform however, mainly because Cortex-M3 based platforms gives us more flexibility with regard to code size.
Also, in my experience, the free toolchains available for CC430 are not as robust as gcc-arm-embedded. TI's msp430-gcc (which supersedes mspgcc),
is still a young effort and there are some known problems with code size optimization.

It is important to know that there are a number of parties who are currently in the process of designing devkits which will be commercially available, 
so the choice should increase in the near future.

Finally, it is of course possible to define your own platform. Especially if you are using an MCU and radio which is already implemented it takes very little effort to add support to OSS-7 for your platform.
Designing your own platform gives you the most flexibility regarding form factor and sensors specific for you use case of course. We are glad to assist in designing a custom board.

##CC1101 RF module for Giant Gecko##

The RF module is a plug-in module for the [Giant Gecko starter kit](https://www.silabs.com/products/mcu/lowpower/Pages/efm32gg-stk3700.aspx) and is developed by University of Antwerp. The STK3700 is a development kit from Silicon Labs, which contains a Cortex M3 chip and several sensors. 
An advantage is that a JLink programmer is already included on the board, making this is a cheap option (currently around 25 euro!). The setup on picture is all you need to start working with the DASH7 open software stack. 
The RF module is based on the Texas Instruments (TI) CC1101 radio. The RF module has two options: First option is to plug it in on a GG, the second option is to plug it in a SmartRF evaluation board from TI . 
Currently there are two baluns available for this module, 433MHz and 868MHz. 

![Gaint Gecko with CC1101 RF module](GG_CC1101.jpg)

Schematics and Eagle files are available in the [git repository](https://github.com/mosaic-lopow/dash7-ap-open-source-stack/tree/master/hardware/stk3700-cc1101). If there are any questions, contact us through the mailing list.


##Wizzikit##

The Wizzikit is developed by WizziLab and is already few years on the market. The package contains two different boards, Wizzibase and  two Wizzimote. 
The boards are based on the CC430F5137, this is a System on Chip (SoC), that integrates an MSP430 microcontroller and the CC1101 radio chip in one IC. 
The programmer is not included in the wizzikit, we recommend to use the TI MSP-FET430UIF programmer or the OLimex MSP430-JTAG-TINY-V2 programmer. 
The DASH7 open software stack supports the Wizzilab platform, however this platform receives less testing since we are not focusing on this platform.

![Wizzikit](WizziKit.png)

[Datasheet (includes schematic)](http://www.wizzilab.com/wp-content/uploads/2013/03/WizziKit2-Datasheet.pdf)


