/*! \file phy.c
 *
 *  \copyright (C) Copyright 2017 University of Antwerp and others (http://oss-7.cosys.be)
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
 */

#include "fec.h"
#include "math.h"
#include "debug.h"
#include "phy.h"

bool phy_radio_channel_ids_equal(const channel_id_t* a, const channel_id_t* b)
{
    //return memcmp(a,b, sizeof(channel_id_t)) == 0; //not working since channel_id_t not packed
    return (a->channel_header_raw == b->channel_header_raw) && (a->center_freq_index == b->center_freq_index);
}

uint16_t phy_calculate_tx_duration(phy_channel_class_t channel_class, phy_coding_t ch_coding, uint8_t packet_length)
{
    double data_rate = 6.0; // Normal rate: 6.9 bytes/tick

    if (ch_coding == PHY_CODING_FEC_PN9)
        packet_length = fec_calculated_decoded_length(packet_length);

    packet_length += sizeof(uint16_t); // Sync word

    switch (channel_class)
    {
    case PHY_CLASS_LO_RATE:
        packet_length += PREAMBLE_LOW_RATE_CLASS;
        data_rate = 1.0; // Lo Rate 9.6 kbps: 1.2 bytes/tick
        break;
    case PHY_CLASS_NORMAL_RATE:
        packet_length += PREAMBLE_NORMAL_RATE_CLASS;
        data_rate = 6.0; // Normal Rate 55.555 kbps: 6.94 bytes/tick
        break;
    case PHY_CLASS_HI_RATE:
        packet_length += PREAMBLE_HI_RATE_CLASS;
        data_rate = 20.0; // High rate 166.667 kbps: 20.83 byte/tick
        break;
#ifdef USE_SX127X
    case PHY_CLASS_LORA:
      assert(false); // TODO
#endif
    }

    // TODO Add the power ramp-up/ramp-down symbols in the packet length?

    return ceil(packet_length / data_rate) + 1;
}
