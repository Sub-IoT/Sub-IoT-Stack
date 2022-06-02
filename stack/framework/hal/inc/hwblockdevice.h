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


#ifndef BLOCKDEVICE_H_
#define BLOCKDEVICE_H_

#include "types.h"
#include "errors.h"

typedef struct blockdevice blockdevice_t;

typedef struct {
  error_t (*init)(blockdevice_t* bd);
  error_t (*read)(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size);
  error_t (*program)(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size);
  error_t (*erase_chip)(blockdevice_t* bd);
  error_t (*erase_block32k)(blockdevice_t* bd, uint32_t addr);
  error_t (*erase_sector4k)(blockdevice_t* bd, uint32_t addr);
  uint32_t erase_block_size;
  uint32_t write_block_size;
} blockdevice_driver_t;

typedef error_t (*blockdevice_erase_t )(blockdevice_t* bd, uint32_t addr);

struct blockdevice {
  blockdevice_driver_t* driver;
  uint32_t size;
  uint32_t offset;
};

error_t blockdevice_init(blockdevice_t* bd);
error_t blockdevice_read(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size);
error_t blockdevice_program(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size);
error_t blockdevice_erase_chip(blockdevice_t* bd, uint32_t addr);
error_t blockdevice_erase_block32k(blockdevice_t* bd, uint32_t addr);
error_t blockdevice_erase_sector4k(blockdevice_t* bd, uint32_t addr);

#endif

