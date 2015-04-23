# Hardware

TODO update 

A goal of OSS-7 is to support different hardware platforms and radio's. The stack provides this portability by using portable C code for the higher layers, and providing a pluggable driver system for hardware specific implementations. There are two kind of drivers: HAL (Hardware Abstraction Layer) and radio drivers.

We currently have the HAL implementations for the following microprocessors:

+ Texas Instruments CC430
+ Texas Instruments MSP430
+ STMicroelectronics STM32L (ARM Cortex-M3 based)

For the radio we support the following radio chips:

+ Texas Instruments CC430
+ Texas Instruments CC1101

There are 2 options to get hardware: either you buy an off the shelf development kit or you need to design and build your own board.

Currently the off the shelf development kit which is best supported is the CC430-based WizziKit from our friends at WizziLab. This kit is available [here](http://www.wizzilab.com/shop/wizzikit/).

The only other option (as far as we know) for off the shelf available devkits which includes everything on one board is the [Texas Instruments EZ430-Chronos](http://processors.wiki.ti.com/index.php/EZ430-Chronos) watch, which is CC430 based as well.

Alternatively you can get started with a development kit for a microprocessor (like [this](http://www.st.com/web/catalog/tools/PF250990) one for STM32L) and attach a radio chip to it using SPI for example.

Designing your own board gives you the most flexibility regarding form factor and sensors specific for you use case of course. We are glad to assist in designing a custom board.

![Tags]({filename}/images/tags.png)