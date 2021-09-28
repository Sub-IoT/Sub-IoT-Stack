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
#include "framework_defs.h"
#include "stm32_device.h"
#include "stm32_common_mcu.h"
#include "stm32_common_eeprom.h"

#include "fs.h"
#include "hwblockdevice.h"
#include "blockdevice_ram.h"

#ifndef PLATFORM_MURATA_ABZ
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_MURATA_ABZ to be defined
#endif

#ifdef USE_SX127X
  // TODO tmp
  #define SX127x_SPI_INDEX  0
  #define SX127x_SPI_PIN_CS  PIN(0, 15)
  #define SX127x_SPI_BAUDRATE 8000000
  #define SX127x_DIO0_PIN PIN(1, 4)
  #define SX127x_DIO1_PIN PIN(1, 1)
  #ifdef PLATFORM_SX127X_USE_DIO3_PIN
    #define SX127x_DIO3_PIN PIN(2, 13)
  #endif
  #ifdef PLATFORM_SX127X_USE_RESET_PIN
    #define SX127x_RESET_PIN PIN(2, 0)
  #endif
  #ifdef PLATFORM_SX127X_USE_VCC_TXCO
	#define SX127x_VCC_TXCO PIN(0, 12)
  #endif
#endif

#define PLATFORM_USE_ABZ // this platform is based on the Murata ABZ module
// Antenna switching uses 3 pins on murata ABZ module
#define ABZ_ANT_SW_RX_PIN PIN(0, 1)
#define ABZ_ANT_SW_TX_PIN PIN(2, 2)
#define ABZ_ANT_SW_PA_BOOST_PIN PIN(2, 1)

// TODO these defines are dependent on how the Murata module is integrated on the board
// we hardcode them for now
#ifdef FRAMEWORK_MODEM_INTERFACE_USE_INTERRUPT_LINES
  #define MODEM2MCU_INT_PIN PIN(0, 11)
  #define MCU2MODEM_INT_PIN PIN(0, 0)
#endif

static blockdevice_stm32_eeprom_t eeprom_bd = (blockdevice_stm32_eeprom_t){
  .base.driver = &blockdevice_driver_stm32_eeprom,
};

extern uint8_t d7ap_volatile_files_data[FRAMEWORK_FS_VOLATILE_STORAGE_SIZE];

static blockdevice_ram_t volatile_bd = (blockdevice_ram_t){
 .base.driver = &blockdevice_driver_ram,
 .base.size = FRAMEWORK_FS_VOLATILE_STORAGE_SIZE,
 .buffer = d7ap_volatile_files_data
};

/** Platform BD drivers*/
/** Platform BD drivers*/
extern blockdevice_t * const metadata_blockdevice;
extern blockdevice_t * const persistent_files_blockdevice;
extern blockdevice_t * const volatile_blockdevice;
#define PLATFORM_METADATA_BLOCKDEVICE metadata_blockdevice
#define PLATFORM_PERMANENT_BLOCKDEVICE persistent_files_blockdevice
#define PLATFORM_VOLATILE_BLOCKDEVICE volatile_blockdevice

#endif
