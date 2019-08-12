/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2019 Aloxy
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
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

#include "fs.h"
#include "hwblockdevice.h"
#include "blockdevice_ram.h"

#ifndef PLATFORM_NATIVE
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_NATIVE to be defined
#endif

// on native we use a RAM blockdevice as NVM as well for now
extern uint8_t d7ap_permanent_files_data[FRAMEWORK_FS_PERMANENT_STORAGE_SIZE];

static blockdevice_ram_t eeprom_bd = (blockdevice_ram_t){
    .base.driver = &blockdevice_driver_ram,
    .size = FRAMEWORK_FS_PERMANENT_STORAGE_SIZE,
    .buffer = d7ap_permanent_files_data
};

extern uint8_t d7ap_volatile_files_data[FRAMEWORK_FS_VOLATILE_STORAGE_SIZE];

static blockdevice_ram_t volatile_bd = (blockdevice_ram_t){
 .base.driver = &blockdevice_driver_ram,
 .size = FRAMEWORK_FS_VOLATILE_STORAGE_SIZE,
 .buffer = d7ap_volatile_files_data
};

/** Platform BD drivers*/
#define PLATFORM_PERMANENT_BD (blockdevice_t*)&eeprom_bd
#define PLATFORM_VOLATILE_BD (blockdevice_t*)&volatile_bd

#endif

