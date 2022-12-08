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

/*! \file fs.h
 * \addtogroup Fs
 * \ingroup framework
 * @{
 * \brief Filesystem APIs used as an abstraction level for underneath FS usage (eg LittleFS)
 * \author	philippe.nunes@cortus.com
 */

#ifndef FS_H_
#define FS_H_

#include <stdint.h>
#include <stdbool.h>

#include "framework_defs.h"
#include "hwblockdevice.h"
    
#ifndef FRAMEWORK_FS_FILE_COUNT
#define FRAMEWORK_FS_FILE_COUNT 70
#endif

#ifndef FRAMEWORK_FS_BLOCKDEVICES_COUNT
#define FRAMEWORK_FS_BLOCKDEVICES_COUNT 3
#endif

#ifndef FRAMEWORK_FS_USER_FILE_COUNT
#define FRAMEWORK_FS_USER_FILE_COUNT 33
#endif

#ifndef FRAMEWORK_FS_PERMANENT_STORAGE_SIZE
#define FRAMEWORK_FS_PERMANENT_STORAGE_SIZE 1900
#endif

#ifndef FRAMEWORK_FS_VOLATILE_STORAGE_SIZE
#define FRAMEWORK_FS_VOLATILE_STORAGE_SIZE 1024
#endif

#define FS_MAGIC_NUMBER { 0x34, 0xC2, 0x00, 0x00 } // first 2 bytes fixed, last 2 byte for version
#define FS_MAGIC_NUMBER_SIZE 4
#define FS_MAGIC_NUMBER_ADDRESS 0

#define FS_NUMBER_OF_FILES_SIZE 4
#define FS_NUMBER_OF_FILES_ADDRESS  4

#define FS_FILE_HEADERS_ADDRESS 8
#define FS_FILE_HEADER_SIZE sizeof(fs_file_t)


typedef enum
{
    FS_BLOCKDEVICE_TYPE_METADATA = 0,
    FS_BLOCKDEVICE_TYPE_PERMANENT = 1,
    FS_BLOCKDEVICE_TYPE_VOLATILE = 2
} fs_blockdevice_types_t;


typedef struct __attribute__((__packed__))
{
    uint8_t blockdevice_index; // the members of fs_blockdevice_types_t are required, but more blockdevices can be registered in the future
    uint32_t length;
    uint32_t addr;
} fs_file_t;

void fs_init();
int fs_init_file(uint8_t file_id, fs_blockdevice_types_t bd_type, const uint8_t* initial_data, uint32_t initial_data_length, uint32_t length);
int fs_read_file(uint8_t file_id, uint32_t offset, uint8_t* buffer, uint32_t length);
int fs_write_file(uint8_t file_id, uint32_t offset, const uint8_t* buffer, uint32_t length);
bool fs_is_file_defined(uint8_t file_id);

uint32_t fs_get_address(uint8_t file_id);

error_t fs_register_block_device(blockdevice_t* block_device, uint8_t bd_index);

#endif /* FS_H_ */

/** @}*/
