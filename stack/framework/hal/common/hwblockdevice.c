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

#include "hwblockdevice.h"
#include "debug.h"

error_t blockdevice_init(blockdevice_t* bd) {
  assert(bd && bd->driver && bd->driver->init);
  return bd->driver->init(bd);
}

error_t blockdevice_read(blockdevice_t* bd, uint8_t *data, uint32_t addr, uint32_t size) {
  assert(bd && bd->driver && bd->driver->read);
  return bd->driver->read(bd, data, addr, size);
}

error_t blockdevice_program(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size) {
  assert(bd && bd->driver && bd->driver->program);
  return bd->driver->program(bd, data, addr, size);
}

error_t blockdevice_erase_chip(blockdevice_t* bd, uint32_t addr){
  assert(bd && bd->driver && bd->driver->erase_chip);
  return bd->driver->erase_chip(bd);
}
error_t blockdevice_erase_block32k(blockdevice_t* bd, uint32_t addr){
  assert(bd && bd->driver && bd->driver->erase_block32k);
  return bd->driver->erase_block32k(bd, addr);
}
error_t blockdevice_erase_sector4k(blockdevice_t *bd, uint32_t addr){
  assert(bd && bd->driver && bd->driver->erase_sector4k);
  return bd->driver->erase_sector4k(bd, addr);
}

