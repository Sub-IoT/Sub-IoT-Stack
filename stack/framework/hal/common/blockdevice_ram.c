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

// This is a blockdevice implementation using a buffer in RAM, mainly used as fallback for platforms which do not have
// a non-volatile memory (driver)

#include "blockdevice_ram.h"
#include "debug.h"
#include "log.h"
#include "string.h"
#include "framework_defs.h"


#if defined(FRAMEWORK_LOG_ENABLED) && defined(HAL_PERIPH_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#define DPRINT_DATA(p, n) log_print_data(p, n)
#else
#define DPRINT(...)
#define DPRINT_DATA(p, n)
#endif

// forward declare driver function pointers
static error_t init(blockdevice_t* bd);
static error_t read(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size);
static error_t program(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size);

blockdevice_driver_t blockdevice_driver_ram = {
    .init = init,
    .read = read,
    .program = program,
    .erase_block_size = 0,          //erase not necessary
    .write_block_size = UINT32_MAX  //blocks don't have a limit to write at once
};


static error_t init(blockdevice_t* bd) {
  blockdevice_ram_t* bd_ram = (blockdevice_ram_t*)bd;
  DPRINT("init RAM block device of size %i\n", bd_ram->size);
  return SUCCESS;
}

static error_t read(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size) {
  blockdevice_ram_t* bd_ram = (blockdevice_ram_t*)bd;
  DPRINT("BD READ %i @ %x\n", size, addr);

  if(size == 0) return SUCCESS;
  if(addr + size > bd_ram->base.size) return -ESIZE;

  memcpy((void*)data, bd_ram->buffer + addr, size);

  return SUCCESS;
}

static error_t program(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size) {
  blockdevice_ram_t* bd_ram = (blockdevice_ram_t*)bd;
  DPRINT("BD WRITE %i @ %x\n", size, addr);

  if(size == 0) return SUCCESS;
  if(addr + size > bd_ram->base.size) return -ESIZE;

  memcpy(bd_ram->buffer + addr, data, size);

  DPRINT_DATA(data, size);

  return SUCCESS;
}
