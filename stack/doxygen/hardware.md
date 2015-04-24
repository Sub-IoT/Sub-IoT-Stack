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

The [EFM32GG_STRK3700](https://www.silabs.com/products/mcu/lowpower/Pages/efm32gg-stk3700.aspx) is currently the most used by us, and thus the best supported.
A disadvantage of this platform is that you need to attach an external CC1101 manually (using a custom expansion board or by wiring).
However, we are working on a PCB design for a devkit containing Giant Gecko and CC1101 as well, which we will opensource shortly.

Currently the only off the shelf devkit available is the WizziKit from WizziLab which can be bought [here](http://www.wizzilab.com/shop/wizzikit/).
At the moment we are not focusing on the CC430 based platform however, mainly because Cortex-M3 based platforms gives us more flexibility with regard to code size.
Also, in my experience, the free toolchains available for CC430 are not as robust as gcc-arm-embedded. TI's msp430-gcc (which supersedes mspgcc),
is still a young effort and there are some known problems with code size optimization.

It is important to know that there are a number of parties who are currently in the process of designing devkits which will be commercially available, 
so the choice should increase in the near future.

Finally, it is of course possible to define your own platform. Especially if you are using an MCU and radio which is already implemented it takes very little effort to add support to OSS-7 for your platform.
Designing your own platform gives you the most flexibility regarding form factor and sensors specific for you use case of course. We are glad to assist in designing a custom board.
