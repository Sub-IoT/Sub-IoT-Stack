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

#include "stm32_device.h"
#include "stm32_common_mcu.h"
#include "platform_defs.h"
#include "stm32_common_eeprom.h"

#include "fs.h"
#include "hwblockdevice.h"
#include "blockdevice_ram.h"

#ifndef PLATFORM_NUCLEO_L073RZ
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_NUCLEO_STM32L152 to be defined
#endif



/********************
 * LED DEFINITIONS *
 *******************/

#define LED0 PIN(0, 5)
#define LED_GREEN 0


/**************************
 * USERBUTTON DEFINITIONS *
 *************************/

#define BUTTON0				PIN(2, 13)

#if defined(USE_SX127X) || defined(USE_NETDEV_DRIVER)
  #define SX127x_SPI_INDEX    0
  #define SX127x_SPI_PIN_CS PIN(1, 6)
  #define SX127x_SPI_BAUDRATE 8000000
  #define SX127x_DIO0_PIN PIN(0, 10)
  #define SX127x_DIO1_PIN PIN(1, 3)
  #ifdef PLATFORM_SX127X_USE_DIO3_PIN
    #define SX127x_DIO3_PIN PIN(1, 4)
  #endif
  #ifdef PLATFORM_SX127X_USE_RESET_PIN
    #define SX127x_RESET_PIN PIN(0, 0)
  #endif
  #ifdef PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN
    #define SX127x_MANUAL_RXTXSW_PIN PIN(2, 1)
  #endif
#endif

#define PLATFORM_NUM_TIMERS 1


/** Platform BD drivers*/
extern blockdevice_t * const metadata_blockdevice;
extern blockdevice_t * const persistent_files_blockdevice;
extern blockdevice_t * const volatile_blockdevice;
#define PLATFORM_METADATA_BLOCKDEVICE metadata_blockdevice
#define PLATFORM_PERMANENT_BLOCKDEVICE persistent_files_blockdevice
#define PLATFORM_VOLATILE_BLOCKDEVICE volatile_blockdevice

#endif
