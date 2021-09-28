/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include "platform_defs.h"

#ifndef PLATFORM_CORTUS_FPGA
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_CORTUS_FPGA to be defined
#endif

#include "machine/gpio.h"
#include "cortus_mcu.h"
#include "hwblockdevice.h"

#define XTAL_FREQ   32000000

/********************
 * LED DEFINITIONS *
 *******************/
#define LED1                PIN(gpioPortA,16);
#define LED2                PIN(gpioPortA,17);
#define LED3                PIN(gpioPortA,18);
#define LED4                PIN(gpioPortA,19);
#define LED5                PIN(gpioPortA,20);
#define LED6                PIN(gpioPortA,21);
#define LED7                PIN(gpioPortA,22);
#define LED8                PIN(gpioPortA,23);

/********************
 *  USB SUPPORT      *
 ********************/

//#define USB_DEVICE

/********************
 * UART DEFINITIONS *
 *******************/

// console configuration
#define CONSOLE_UART        0
#define CONSOLE_LOCATION    1
#define CONSOLE_BAUDRATE    115200

/*************************
 * DEBUG PIN DEFINITIONS *
 ************************/

//#define DEBUG_PIN_NUM 4
//#define DEBUG0	D4
//#define DEBUG1	D5
//#define DEBUG2	D6
//#define DEBUG3	D7

/**************************
 * USERBUTTON DEFINITIONS *
 *************************/
#define BUTTON0             PIN(gpioPortA, 4)
#define BUTTON1             PIN(gpioPortA, 5)

// CC1101 PIN definitions
#ifdef USE_CC1101
#define CC1101_SPI_USART    1  // not used
#define CC1101_SPI_BAUDRATE 8  // divider
#define CC1101_SPI_PIN_CS   PIN(gpioPortA, 2)
#define CC1101_GDO0_PIN     PIN(gpioPortA, 0)
#define CC1101_GDO2_PIN     PIN(gpioPortA, 1)
#endif

#if defined(USE_SX127X) || defined(USE_NETDEV_DRIVER)
#define SX127x_SPI_INDEX    1
#define SX127x_SPI_PIN_CS   PIN(gpioPortA, 2)
#define SX127x_SPI_BAUDRATE 8 //10000000
#define SX127x_DIO0_PIN     PIN(gpioPortA, 0)
#define SX127x_DIO1_PIN     PIN(gpioPortA, 1)
#endif


/** Platform BD drivers*/
extern blockdevice_t * const metadata_blockdevice;
extern blockdevice_t * const persistent_files_blockdevice;
extern blockdevice_t * const volatile_blockdevice;
#define PLATFORM_METADATA_BLOCKDEVICE metadata_blockdevice
#define PLATFORM_PERMANENT_BLOCKDEVICE persistent_files_blockdevice
#define PLATFORM_VOLATILE_BLOCKDEVICE volatile_blockdevice


#endif
