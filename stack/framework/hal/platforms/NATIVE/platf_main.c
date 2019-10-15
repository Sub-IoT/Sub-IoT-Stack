/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
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

#include "bootstrap.h"
#include "hwgpio.h"
#include "hwleds.h"
#include "hwsystem.h"
#include "debug.h"
#include "hwdebug.h"
#include "hwradio.h"
#include "errors.h"
#include "blockdevice_ram.h"
#include "framework_defs.h"

#define METADATA_SIZE (4 + 4 + (12 * FRAMEWORK_FS_FILE_COUNT))

// on native we use a RAM blockdevice as NVM as well for now
extern uint8_t d7ap_fs_metadata[METADATA_SIZE];
extern uint8_t d7ap_files_data[FRAMEWORK_FS_PERMANENT_STORAGE_SIZE];
extern uint8_t d7ap_volatile_files_data[FRAMEWORK_FS_VOLATILE_STORAGE_SIZE];

static blockdevice_ram_t metadata_bd = (blockdevice_ram_t){
    .base.driver = &blockdevice_driver_ram,
    .size = METADATA_SIZE,
    .buffer = d7ap_fs_metadata
};

static blockdevice_ram_t permanent_bd = (blockdevice_ram_t){
    .base.driver = &blockdevice_driver_ram,
    .size = FRAMEWORK_FS_PERMANENT_STORAGE_SIZE,
    .buffer = d7ap_files_data
};

static blockdevice_ram_t volatile_bd = (blockdevice_ram_t){
    .base.driver = &blockdevice_driver_ram,
    .size = FRAMEWORK_FS_VOLATILE_STORAGE_SIZE,
    .buffer = d7ap_volatile_files_data
};

blockdevice_t * const metadata_blockdevice = (blockdevice_t* const) &metadata_bd;
blockdevice_t * const persistent_files_blockdevice = (blockdevice_t* const) &permanent_bd;
blockdevice_t * const volatile_blockdevice = (blockdevice_t* const) &volatile_bd;


void __platform_init()
{
    blockdevice_init(metadata_blockdevice);
    blockdevice_init(persistent_files_blockdevice);
    blockdevice_init(volatile_blockdevice);
}

void __platform_post_framework_init()
{
}

int main()
{
    //initialise the platform itself
    __platform_init();
    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
    //initialise platform functionality that depends on the framework
    __platform_post_framework_init();

    scheduler_run();
    return 0;
}

