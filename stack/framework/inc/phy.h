/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2017 University of Antwerp
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

#include "types.h"
#include "platform_defs.h"
#include "dae.h"

/*!
 * \file phy.h
 * \addtogroup phy
 * \ingroup framework
 * @{
 * \brief The PHY module provides PHY functions not specific to one chip
 * \author glenn.ergeerts@uantwerpen.be
 */

#ifndef _PHY_H_
#define _PHY_H_

/* \brief The syncword classes as defined in D7AP
 *
 */
typedef enum
{
    PHY_SYNCWORD_CLASS0 = 0x00,
    PHY_SYNCWORD_CLASS1 = 0x01
} phy_syncword_class_t;

typedef enum
{
    PREAMBLE_LOW_RATE_CLASS = 4, //(4 bytes, 32 bits)
    PREAMBLE_NORMAL_RATE_CLASS = 4, //(4 bytes, 32 bits)
    PREAMBLE_HI_RATE_CLASS = 6, //(6 bytes, 48 bits)
} phy_preamble_min_length_t;

/* \brief Utility function to check whether two channel_id_t are equal
 *
 * \param a	The first channel_id
 * \param b	The second channel_id
 * \return bool	true if the two channel_id are equal, false otherwise.
 */
bool phy_radio_channel_ids_equal(const channel_id_t* a, const channel_id_t* b);

uint16_t phy_calculate_tx_duration(phy_channel_class_t channel_class, phy_coding_t ch_coding, uint8_t packet_length, bool payload_only);

#endif //_PHY_H_

/** @}*/
