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
 *  \author philippe.nunes@cortus.com
 *
 */

#include <stdint.h>

#include "pn9.h"

void pn9_next(uint16_t *last)
{
    uint16_t pn9_new;
    pn9_new =  (((*last & 0x20) >> 5) ^ *last) << 8;
    pn9_new |= (*last >> 1) & 0xff;
    *last = pn9_new & 0x1ff;
}

uint16_t pn9_generator(uint16_t *pn9)
{
    int i;

    for (i=0; i<8; i++) {
        pn9_next(pn9);
    }
    return *pn9;
}

void pn9_encode(uint8_t *data, uint16_t length)
{
    uint16_t pn9 = PN9_INITIALIZER; //// LFSR initialised to the specified polynomial
    uint8_t *p = data;

    for (; p < data + length; p++) {
        *p ^= pn9;
        pn9_generator(&pn9);
    }
}
