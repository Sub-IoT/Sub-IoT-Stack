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

#ifndef __BLOCKDEVICE_RAM_H_
#define __BLOCKDEVICE_RAM_H_

#include "hwblockdevice.h"

// This is a blockdevice implementation using a buffer in RAM, mainly used as fallback for platforms which do not have
// a non-volatile memory (driver)

// extend blockdevice_t
typedef struct {
  blockdevice_t base;
  uint8_t* buffer;
} blockdevice_ram_t;

extern blockdevice_driver_t blockdevice_driver_ram;
#endif //__BLOCKDEVICE_RAM_H_
