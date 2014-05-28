# Introduction

The goal of the project is to provide a reference implementation of the DASH7 Alliance protocol.
This implementation should focus on completeness, correctness and being easy to understand.
Performance and code size are less important aspects.
For clarity a clear separation between the ISO layers is maintained in the code.

This project is built with CCS v5.3 and currently only supports the WizziMote and ArtesisTag.
(As discussed previously supporting multiple toolchains and platforms is something we will do later.)

The PHY directory contains the PHY API together with radio specific implementations.
At the moment there are 2 implementations, one for the CC430 radio and a stub one which only does logging.
The implementation to be used should come from some kind of configure step later on, but is now defined using precompiler directives in d7aoss.h .

Further up in the stack are the DLL layer, the transport layer and the network layer.

Next to this layer there is also a hardware abstraction layer.
The HAL directory should contain the hardware abstraction layer API and its implementations. At the moment this contains code from TI's driverlib and some functions for button, leds, rtc, system and uart handling. Support for multiple hardware platforms is not implemented at the moment and should be discussed eventually.
There is also a basic logging functionality defined in log.h which allows logging to UART. (This should be expanded to allow specifying for example the layer and the log level.)

The examples directory contain example programs showing specific functionality of the stack.

The tools/logger directory contains both a command line application and a GUI (Qt) application which outputs the logging received over UART in a formatted way.

Please commit changes to a separate branch and create a pull request if you want to have it merged to the master branch.

License: LGPS v2.1 http://www.gnu.org/licenses/lgpl-2.1.txt

# Getting started

Wiki: https://github.com/CoSys-Lab/dash7-ap-open-source-stack/wiki/Start-To-Guide

# Next steps

* implement complete spec
* resolve TODOs (which are everywhere atm)
* add documentation comments
* fix coding style everywhere


[![githalytics.com alpha](https://cruel-carlota.pagodabox.com/725ca2b507c1d85404803b081f882cab "githalytics.com")](http://githalytics.com/CoSys-Lab/dash7-ap-open-source-stack)