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
#include "stm32_device.h"
#include "stm32_common_mcu.h"
#include "stm32_common_eeprom.h"

#include "fs.h"
#include "hwblockdevice.h"
#include "blockdevice_ram.h"

#ifndef PLATFORM_OCTA-L072Z
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_OCTA-L072Z to be defined
#endif

/********************
 * LED DEFINITIONS *
 *******************/

#define LED1 PIN(1, 5)
#define LED2 PIN(1, 6)
#define LED3 PIN(1, 7)


/**************************
 * USERBUTTON DEFINITIONS *
 *************************/

#define BUTTON0           PIN(1, 2)

#define SPI2_PIN_CS  PIN(1, 12)


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
#define PLATFORM_PERMANENT_BD (blockdevice_t*)&eeprom_bd
#define PLATFORM_VOLATILE_BD (blockdevice_t*)&volatile_bd


#endif
