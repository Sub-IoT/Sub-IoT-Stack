# Overview

The native platform allows the stack to be run on a Linux computer.  The
emulation presently supports:

- Push button emulation with 10 virtual buttons using numbered keys 0-9
- Unlimited mumber of sensor application instances
- Console serial output to the same terminal the application is launched
- Emulated radio based on multi-cast IP communications
- Emulated UART in conjunction with the socat tool to allow pyd7a domain
  to interact with the gateway using the serial modem interface
- Option to connect to real peripherals (eg GPS) using a real UART
  on your Linux machine via the emulated UART layer

# Why is this useful?

- Access to physical hardware is not always possible
- Allow application and stack to be tested independently of hardware
- Full stack debug trace can be CPU intensive on small embedded CPUs
- More rapidly try out stack or application ideas

# Quick start

Launch virtual serial ports using socat as follows:

socat pty,link=/tmp/S0,raw,echo=0 pty,link=/tmp/S1,raw,echo=0 &

Run the following command to build using the assigned device /tmp/S0 from socat above:

cmake <stack_directory> -DCMAKE_TOOLCHAIN_FILE=<stack_directory>/cmake/toolchains/gcc.cmake -DAPP_GATEWAY=y -DAPP_SENSOR_PUSH=y -DAPP_SENSOR_PULL=y -DPLATFORM_UART_DEV0=/tmp/S0

Launch your applications in separate terminal windows:

./apps/gateway/gateway.elf
./apps/sensor_pull/sensor_pull/elf

You can now interact with the gateway using pyd7a and the assigned device /tmp/S1 from socat above:

PYTHONPATH=`pwd` python examples/query_nodes.py -d /tmp/S1

# Related repositories

[pyd7a](https://github.com/MOSAIC-LoPoW/pyd7a) provides a collection of python modules, supporting the DASH7 Alliance Protocol in general, and OSS-7 in particular.
