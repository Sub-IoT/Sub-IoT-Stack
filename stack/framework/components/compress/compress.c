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
 */

#include <stdint.h>
#include "debug.h"
#include "compress.h"

uint8_t compress_data(uint16_t value, bool ceil)
{
    uint8_t mantissa;
    uint16_t remainder;

    for ( int i = 0; i < 8; i++)
    {
        if (value <= (pow(4, i) * 31))
        {
            mantissa = value / pow(4, i);
            remainder = value % (uint16_t)(pow(4, i));

            if (ceil && remainder)
                mantissa++;

            return (uint8_t)( i<<5 | mantissa);
        }
    }
    return 0;
}
