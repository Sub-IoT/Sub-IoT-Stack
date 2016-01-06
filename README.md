# Welcome

OSS-7 is an open source implementation of the [DASH7 Alliance](http://www.dash7-alliance.org) protocol for ultra low power wireless sensor communication. The aim of the project is to provide a reference implementation of the protocol stack which allows for fast development and prototyping of DASH7 based products. This implementation focusses on completeness, correctness, ease of use and understanding. Performance and code size are less important aspects. For clarity a clear separation between the ISO layers is maintained in the code.

For more information visit the [OSS-7 site](http://mosaic-lopow.github.io/dash7-ap-open-source-stack/) and our [doxygen site](http://mosaic-lopow.github.io/dash7-ap-open-source-stack/doxygen/)

&copy; Copyright 2015-2016, University of Antwerp and others

Licensed under the Apache License, Version 2.0 (the "License"): You may not use these files except in compliance with the License. You may obtain a copy of the License at [http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

## Getting Started

OSS-7 uses `cmake` to build the demo applications and drivers for the supported platforms. You can use it interactively, as discussed on our [doxygen site](http://mosaic-lopow.github.io/dash7-ap-open-source-stack/doxygen/md_gettingstarted.html). Most of these steps are also combined in a `Makefile`. Overriding only a couple of environment variables allows for a quick compilation of one of the supported applications:

### Default Application: Gateway

```bash
$ cd dash7-ap-open-source-stack/
$ make
*** preparing ../build/gateway
-- Cross-compiling using gcc-arm-embedded toolchain
-- Cross-compiling using gcc-arm-embedded toolchain
-- detected supported platforms: EFM32GG_STK3700 EFM32HG_STK3400 ifest matrix_tp1089
-- selected platform: EFM32GG_STK3700
-- The ASM compiler identification is GNU
-- Found assembler: /Users/xtof/Workspace/UA/projects/gcc-arm-none-eabi-4_9-2015q3/bin/arm-none-eabi-gcc
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/xtof/Workspace/UA/projects/build/gateway
*** building ../build/gateway/apps/gateway/gateway.elf
Scanning dependencies of target CHIP_CC1101
[  1%] Building C object framework/hal/platforms/platform/chips/cc1101/CMakeFiles/CHIP_CC1101.dir/cc1101.c.obj
...
Scanning dependencies of target gateway.elf
[ 98%] Building C object apps/gateway/CMakeFiles/gateway.elf.dir/app.c.obj
[100%] Linking C executable gateway.elf
[100%] Built target gateway.elf
```

### Platforms

As can be seen in the output of the `cmake` build process, several platforms are supported:

```
-- detected supported platforms: EFM32GG_STK3700 EFM32HG_STK3400 ifest matrix_tp1089
```

These platforms can be found in the `stack/framework/hal/platforms` folder. They define and provide access to the available hardware components.

The default platform is `EFM32GG_STK3700`. Specifying a different platform is possible using the `PLATFORM` environment variable:

```bash
$ PLATFORM=EFM32HG_STK3400 make
*** preparing ../build/gateway
-- Cross-compiling using gcc-arm-embedded toolchain
-- Cross-compiling using gcc-arm-embedded toolchain
-- detected supported platforms: EFM32GG_STK3700 EFM32HG_STK3400 ifest matrix_tp1089
-- selected platform: EFM32HG_STK3400
-- The ASM compiler identification is GNU
-- Found assembler: /Users/xtof/Workspace/UA/projects/gcc-arm-none-eabi-4_9-2015q3/bin/arm-none-eabi-gcc
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/xtof/Workspace/UA/projects/build/gateway
*** building ../build/gateway/apps/gateway/gateway.elf
...
Scanning dependencies of target gateway.elf
[ 98%] Building C object apps/gateway/CMakeFiles/gateway.elf.dir/app.c.obj
[100%] Linking C executable gateway.elf
[100%] Built target gateway.elf
```

**A WORD OF CAUTION** - We recently changed our UART, SPI and I2C implementations. Although done with care, things can still go wrong in unexpected ways. Just use the bug reporting facilities to inform us, of help out and submit a pull request. The `EFM32GG_STK3700` is well tested, on the others YMMV.

### Specifying a Different Application

Demo Applications are located in the `stack/apps` folder:

```bash
$ ls stack/apps/
CMakeLists.txt  d7ap_test/      noise_logger/   phy_test/       simple_leds/
continuous_tx/  gateway/        per_test/       radio_ping/     stk3700_sensor/
```

Any of these apps can be selected using tha `APP` environment variable:

```bash
$ APP=per_test make
*** preparing ../build/per_test
-- Cross-compiling using gcc-arm-embedded toolchain
-- Cross-compiling using gcc-arm-embedded toolchain
-- detected supported platforms: EFM32GG_STK3700 EFM32HG_STK3400 ifest matrix_tp1089
-- selected platform: EFM32GG_STK3700
-- The ASM compiler identification is GNU
-- Found assembler: /Users/xtof/Workspace/UA/projects/gcc-arm-none-eabi-4_9-2015q3/bin/arm-none-eabi-gcc
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/xtof/Workspace/UA/projects/build/per_test
*** building ../build/per_test/apps/per_test/per_test.elf
...
Scanning dependencies of target per_test.elf
[ 98%] Building C object apps/per_test/CMakeFiles/per_test.elf.dir/per_test.c.obj
[100%] Linking C executable per_test.elf
[100%] Built target per_test.elf
```

**A WORD OF CAUTION** - We recently changed our UART, SPI and I2C implementations. Although done with care, things can still go wrong in unexpected ways. Just use the bug reporting facilities to inform us, of help out and submit a pull request.
