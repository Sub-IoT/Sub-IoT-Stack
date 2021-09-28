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
 * \author maarten.weyn@uantwerpen.be
 *
 */

/*! \file crc.h
 * \addtogroup crc
 * \ingroup framework
 * @{
 * \brief Implements the CRC calculation according the CCITT CRC16 polynomial
 * \author maarten.weyn@uantwerpen.be
 */

#ifndef CRC_H_
#define CRC_H_


#include <stdint.h>

uint16_t crc_calculate(uint8_t* data, uint8_t length);

#endif /* CRC_H_ */

/** @}*/
