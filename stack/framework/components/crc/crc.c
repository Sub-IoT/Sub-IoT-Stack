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
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include <stdint.h>

#include "crc.h"
#include "ng.h"

static uint16_t NGDEF(_crc);
#define crc NG(_crc)

static void update_crc(uint8_t x)
{
     uint16_t crc_new = (uint8_t)(crc >> 8) | (crc << 8);
     crc_new ^= x;
     crc_new ^= (uint8_t)(crc_new & 0xff) >> 4;
     crc_new ^= crc_new << 12;
     crc_new ^= (crc_new & 0xff) << 5;
     crc = crc_new;
}

uint16_t crc_calculate(uint8_t* data, uint8_t length)
{
    crc = 0xffff;
    uint8_t i = 0;

    for(; i<length; i++)
    {
        update_crc(data[i]);
    }
    return crc;
}
