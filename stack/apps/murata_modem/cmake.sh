#!/bin/sh
TOOLCHAIN_DIR=/home/farouk/Desktop/Workspace/Toolchains/gcc-arm/

BASEDIR=$(dirname "$0")

if [ -z "$TOOLCHAIN_DIR" ]
then
    echo "gcc-arm toolchains directory not set! Please set TOOLCHAIN_DIR!"
    exit
fi
cd $BASEDIR../../../../
mkdir -p build
cd build
cmake ../stack/ -DAPP_MURATA_MODEM=y -DPLATFORM=MURATA_ABZ -DMODULE_LORAWAN=y -DTOOLCHAIN_DIR=$TOOLCHAIN_DIR -DMODULE_D7AP_PACKET_QUEUE_SIZE=3 -DFRAMEWORK_SHELL_ENABLED=y -DFRAMEWORK_CONSOLE_ENABLED=y -DPLATFORM_CONSOLE_UART=0 -DPLATFORM_MODEM_INTERFACE_UART=1
