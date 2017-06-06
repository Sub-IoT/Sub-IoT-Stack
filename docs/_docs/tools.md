---
title: Tools
permalink: /docs/tools/
---

# pyd7a

Make sure to checkout [pyd7a](https://github.com/MOSAIC-LoPoW/pyd7a) as well. This python package provides a collection of python modules, supporting the DASH7 Alliance Protocol in general, and OSS-7 in particular. It has an API to programmatically access an oss7 modem, tools to parse incoming packets and a webgui to interact with a modem and the network behind it.

# IDE Support

The OSS-7 buildsystem and code does not require a specific IDE. The user can choose which IDE to use, if any.
CMake supports generating project files for Eclipse CDT, CodeBlocks etc, instead of plain makefiles using the -G option.
Alternatively you can use an IDE which natively uses cmake projects like Qt Creator or JetBrain's CLion.

# Debugging

For debugging most toolchains come with a GDB client which you can attach to a GDB server which is compatible with your programmer.
For instance ARM Cortex-M platforms can use Segger's JLink programmers which comes with JLinkGDBServer. The arm-gcc-embedded toolchain
can then be used to connect with the JLinkGDBServer for flashing and debugging. Segger also provides the [Ozone](https://www.segger.com/ozone.html) GUI tool for debugging.

For Eclipse user: [here](http://gnuarmeclipse.livius.net/) you can find an Eclipse plugin to integrate JLink debugging in Eclipse.
Similarly, [EmbSysRegView](http://embsysregview.sourceforge.net/) is an Eclipse addin to view the registers of different MCU's.

We don't go into more detail about this here since this depends a lot on your favorite tools and the platform you are using.
