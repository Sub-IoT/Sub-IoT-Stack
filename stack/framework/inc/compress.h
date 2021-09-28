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
 * \author philippe.nunes@cortus.com
 *
 */

/*!
 * \file compress.h
 * \addtogroup compress
 * \ingroup framework
 * @{
 * \brief Implements the compressed format
 *
 * The compressed format allows compressing a unit ranged from 0 to 507904 to 1 byte with variable resolution.
 *
 * It can be converted back to units using the formula T = (4^EXP)·(MANT).
 * \author philippe.nunes@cortus.com
 */

#ifndef COMPRESS_H_
#define COMPRESS_H_

#include <stdbool.h>
#include "math.h"

typedef union{
  uint8_t raw;
  struct {
    uint8_t exp : 3; // Exponent (ranging from 0 to 7)
    uint8_t mant : 5; // Mantissa (randing from 0 to 31)
  };
} compressed_time_t;

#define CT_DECOMPRESS(ct) (pow(4, ct >> 5) * (ct & 0b11111))

uint8_t compress_data(uint16_t value, bool ceil);

#endif /* COMPRESS_H_ */

/** @}*/
