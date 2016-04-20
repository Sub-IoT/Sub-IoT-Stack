# Hardware

## Platforms

A goal of OSS-7 is to support different hardware platforms and radio's. 
The stack provides this portability by using portable C code for the higher layers, and providing a pluggable driver system for hardware specific implementations. 
As explained in the [architecture documentation](architecture.md) OSS-7 has the concept of chips and platforms inside the hardware abstraction layer.
A chip implementation contains drivers for MCU peripherals or radio chips. A platform is a combination of an MCU and a radio chips and describes the board wiring and features like LEDs or buttons. 

Currently we support the following plaforms, in decreasing order of completeness/stability:

Platform        | MCU                                   | Radio                         | Toolchain         | 
--------------- | ------------------------------------- | ----------------------------- | ----------------- | 
EFM32GG_STK3700 | Silicon Labs Giant Gecko (Cortex-M3)  | Texas Instruments CC1101      | gcc-arm-embedded  |
EFM32HG_STK3400 | Silicon Labs Happy Gecko (Cortex-M0+) | Texas Instruments CC1101      | gcc-arm-embedded  |
wizzimote       | Texas Instruments CC430 (MSP430)      | Texas Instruments CC1101 (SoC)| msp430-gcc        |
EZR32LG_WSTK6200| Silicon Labs EZR32LG SoC (Cortex-M3)	| EZradio si4460 			| gcc-arm-embedded  |

The [EFM32GG_STK3700](https://www.silabs.com/products/mcu/lowpower/Pages/efm32gg-stk3700.aspx) is currently the most used by us, and thus the best supported.
A disadvantage of this platform is that you need to attach an external CC1101. We designed a CC1101-based module which can be plugged in the expansion port of the devkit, see below for the schematics.
Next to this, we are working on a PCB design for a devkit containing Giant Gecko and CC1101 as well, which we will opensource shortly.

The [EFM32HG_STK3400](https://www.silabs.com/products/mcu/32-bit/Pages/efm32hg-stk3400.aspx) is very similar to the STK3700 but instead has a Cortex-M0+ instead of Cortex-M3 and a more capable LCD screen. The same CC1101 module as used for the STK3700 can be plugged into the expansion header.

##EZR32LG_WSTK6200##
The [EZR32LG_WSTK6200](https://www.silabs.com/products/wireless/wirelessmcu/Pages/ezr32lg-starter-kits.aspx) platform is a starter kit based on the EZR32 Leopard Gecko Wireless MCU. This SoC contains a Wonder Gecko Cortex-M3 combined with an RF chip (si4460). 

![The EZR32LG devkit](wstk6200.png)

CMAKE Settings

To use the default UART using Breakout pad P4 (TX) and P6 (RX):
* PLATFORM_EZ32LG_WSTK6200A_CONSOLE_UART		0
* PLATFORM_EZ32LG_WSTK6200A_CONSOLE_LOCATION	1

Connect P4 to Yellow wire of FTDI connector
Connect P6 to Red wire of FTDI connector (if this wire is connected, make sure you keep the board powered)
Connect GND to Black wire of the FTDI connector
You can power the boad using the Red wire of the FTID using the 5V!!! pin of the dev kit.

To use the VCOM (use ethernet port and telnet (port 4901)):
* PLATFORM_EZ32LG_WSTK6200A_CONSOLE_UART		3
* PLATFORM_EZ32LG_WSTK6200A_CONSOLE_LOCATION	1
* PLATFORM_USE_VCOM								ENABLE

Sensors
initSensors() will initialize the Humidity and Temperature sensor on the devkit.
After initialization the values can be read using getHumidityAndTemperature()

## Other

It is important to know that there are a number of parties who are currently in the process of designing devkits which will be commercially available, 
so the choice should increase in the near future.

At the moment we are not focusing on the CC430 based platform however, mainly because ARM Cortex based platforms gives us more flexibility with regard to code size.
Also, in my experience, the free toolchains available for CC430 are not as robust as gcc-arm-embedded. TI's msp430-gcc (which supersedes mspgcc),
is still a young effort and there are some known problems with code size optimization. While OSS7 has a HAL implementation to supports the Wizzilab platform, this platform is not teste by us anymore since we stopped using cc430 based hardware for our projects. Furthermore, we are aware of crashes when running the current OSS7 stack on cc430. Only the PHY layer is validated to work on cc430 for now. We currently lack time to look into this and would like to focus on EFM32, but we would ofcourse welcome patches fro people who would still like to use cc430.

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


![Wizzikit](WizziKit.png)

[Datasheet (includes schematic)](http://www.wizzilab.com/wp-content/uploads/2013/03/WizziKit2-Datasheet.pdf)


