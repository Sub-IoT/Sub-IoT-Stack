# Getting Started

[TOC]

TODO update

## Get some hardware

For an overview of supported hardware, [look here]({filename}/pages/hardware.md)

## Get the source code

The code is hosted on [github](https://github.com/CoSys-Lab/dash7-ap-open-source-stack/commits/master), so either fork or clone the repository.

## Using Texas Instruments Code Composer Studio Toolchain

This section applies only if you want to use TI CCS for programming MSP430/CC430 based nodes. If you want to use a gcc-based toolchain or are using another hardware platform skip over to the next section. 

### Install Texas Instruments CCS

TI CSS can be downloaded [here](http://www.ti.com/tool/ccstudio). 

For OSS-7, the free (16 KB code limit for MSP430) can be used.

### Import the d7aoss library in CCS
* create new CCS project
 * use project name "d7aoss" and output type "static library"
 * do not use the default location but point it to /d7aoss in the repository
 * select CC430F5137 as device, or the one suited for your hardware
 * select the empty project template and click finish
* when using a cc430, choose the correct platform in d7aoss.h
* in the phy directory include only the used radio in the build, exclude all others (e.g. stub)
  (right click the folder - Build - Exclude resource from build)
* in the hal directory include only the used hardware in the build, exclude all others (e.g. stub)
* build the project

### Test Example Application
For this example the star network example will be used.

#### Endpoint node

* Create a new CCS Project
 * Name: star_push_endpoint_example
 * Output Type: Executable
 * As location, use examples\star_push_endpoint_example of the repository
 * Select the correct device
 * Select "Empty Project"
* Configure application to use d7aoss
 * go to project properties (right click on project - properties)
 * Build - Dependencies - Add - d7aoss
 * Build - MSP430 Compiler - Include Options - Add Icon - Workspace - d7aoss
* Configure linker
 * open lnk_cc430f*.cmd
 * add ` -l "../../../d7aoss/Debug/d7aoss.lib"` at the end of the file the path should point to the d7aoss.lib file created by the d7aoss project

#### Gateway node

Do exactly the same for the examples\star_push_gateway_example, name this project star_push_gateway_example

### Program the nodes

Program the endpoint to one or more nodes, program the gateway to an other one.
Currently we use the [Olimex MSP430-JTAG-TINY-V2](https://www.olimex.com/Products/MSP430/JTAG/MSP430-JTAG-TINY-V2/).

## Using the cmake buildsystem for gcc based toolchains

OSS-7 uses [CMake](http://www.cmake.org/) to configure the buildproces settings and toolchain. The following CMake variables should be defined when generating a buildscript:

  Variable        | Description	| Possible values  
 ------------- |-------------| -----
 DD7AOSS_PHY_RADIO_DRIVER | defines which radio driver to use | cc430, cc1101, stub, sim
 D7AOSS_HAL_DRIVER | defines which hardware abastraction layer implementation to use      | cc430, msp430, stm32l, stub, sim
 D7AOSS_BUILD_EXAMPLES | setting the flag build optional examples instead of only the stack    |  y, n 
 CMAKE_TOOLCHAIN_FILE | this files specifies the compiler toolchain to use and specific compiler options | see cmake/toolchains/*.cmake files

You can specify the values of these parameters by using -D arguments on the commandline or by running cmake-gui in the build directory.

Here is an example using CC430, assuming you cloned the repository in the directory oss-7 are inside this directory:

```bash
$ cd ..
$ mkdir oss7-build && cd oss7-build
$ cmake ../oss-7 -DD7AOSS_PHY_RADIO_DRIVER="cc430" -DD7AOSS_HAL_DRIVER="cc430" -DD7AOSS_BUILD_EXAMPLES=y -DCMAKE_TOOLCHAIN_FILE="../oss-7/cmake/toolchains/mspgcc.cmake"
$ make
```
## CC430 board configuration

For CC430 there is a (preliminary) board configuration method. Configurations are defined in header files in `d7aoss/hal/cc430/platforms/` . Choosing your platform is done be defining it in platform.h in the same directory.

This will be replaced by a cmake based system which also works for non CC430 boards later.

## Debug / Log

* You can use the d7-oss-logger in tools\PyLogger to log debug or application data. 
* You can use the file d7aoss\framework\log.h to define the loglevel of the framework. This will be replaced by a cmake based system later.
* With the star_push_gateway_example you will not see any information if you do not enable any logging in log.h, since the application itself does not use the methods in log.h
* You can use the d7-logger in tools\pylogger_rss to log the data of the star_push_gateway_example.
* Tracing can be enabled by project properties - Build - MSP430-Compiler - Advanced Options - Entry/Exit Hook Options - select name in both entry_param and exit_param.

If you use the logger on the gateway of the example and have one endnode programmed you should have an output similar to:

![Logger]({filename}/images/logger.png)

[![githalytics.com alpha](https://cruel-carlota.pagodabox.com/725ca2b507c1d85404803b081f882cab "githalytics.com")](http://githalytics.com/CoSys-Lab/dash7-ap-open-source-stack)