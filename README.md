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

* clone the repository
* import the d7aoss library in CCS:
    * create new CCS project 
    * use project name "d7aoss" and output type "static library"
    * do not use the default location but point it to <repo root>/d7aoss
    * select CC430F5137 as device
    * select the empty project template and click finish
    * choose the correct platform in hal/platforms/platform.h
    * in the phy directory include only the used radio in the build, exclude all others
    * in the hal directory include only the used hardware in the build, exclude all others
    * the d7aoss project should be created an building it should work
* import applications
    * create new CCS project
    * set the project name to the application name (eg phy_test) and output type "executable"
    * do not use the default location but point it to the right directory, eg <repo root>/examples/phy_test
    * select CC430F5137 as device
    * select the empty project template and click finish
    * add dependencies: project properties | build | Dependencies | Add | d7aoss
    * add include dir: project properties | build | MSP430 compiler | include options | add dir to include search path | workspace | d7aoss
    * the application should now compile and link

# Next steps

* implement complete spec
* resolve TODOs (which are everywhere atm)
* add documentation comments
* fix coding style everywhere
* add copyright/license header


