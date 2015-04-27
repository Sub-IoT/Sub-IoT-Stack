/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file packet.h
 * \ingroup D7AP
 * @{
 * \brief This module contains data structures and behaviour for a D7AP 'packet' when moving through all layers of the stack
 * \author glenn.ergeerts@uantwerpen.be
 */

#ifndef OSS_7_PACKET_H
#define OSS_7_PACKET_H

#include <hwradio.h>

/*! \brief A D7AP 'packet' used over all layers of the stack. Contains both the raw packet data (as transmitted over the air) as well
 * as metadata parsed or generated while moving through the different layers */
typedef struct
{
    hw_radio_packet_t hw_radio_packet; // TODO we might not need all metadata included in hw_radio_packet_t. If not copy needed data fields
    uint8_t data[255]; // reserves space for hw_radio_packet_t.data flexible array member // TODO configure max length from cmake
} packet_t;


void packet_init(packet_t*);

#endif //OSS_7_PACKET_H

/** @}*/
