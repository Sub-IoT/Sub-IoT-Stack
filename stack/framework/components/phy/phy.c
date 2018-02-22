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

#define LORA_T_SYMBOL_SF9_MS 4.096 // based on SF9 and 125k BW
#define LORA_T_PREAMBE_SF9_MS (8 + 4.25) * LORA_T_SYMBOL_SF9_MS // assuming 8 symbols for now

bool phy_radio_channel_ids_equal(const channel_id_t* a, const channel_id_t* b)
{
    //return memcmp(a,b, sizeof(channel_id_t)) == 0; //not working since channel_id_t not packed
    return (a->channel_header_raw == b->channel_header_raw) && (a->center_freq_index == b->center_freq_index);
}

uint16_t phy_calculate_tx_duration(phy_channel_class_t channel_class, phy_coding_t ch_coding, uint8_t packet_length, bool payload_only)
{
    double data_rate = 6.0; // Normal rate: 6.9 bytes/tick

    if (ch_coding == PHY_CODING_FEC_PN9)
        packet_length = fec_calculated_decoded_length(packet_length);

    if(!payload_only)
      packet_length += sizeof(uint16_t); // Sync word

#ifdef USE_SX127X
    if(channel_class == PHY_CLASS_LORA) {
        // based on http://www.semtech.com/images/datasheet/LoraDesignGuide_STD.pdf
        // only valid for explicit header, CR4/5, SF9 for now
        uint16_t payload_symbols = 8 + ceil(2*(packet_length+1)/9)*5;
        uint16_t packet_duration = LORA_T_PREAMBE_SF9_MS + payload_symbols * LORA_T_SYMBOL_SF9_MS;
        return packet_duration;
    }
#endif

    switch (channel_class)
    {
    case PHY_CLASS_LO_RATE:
        if(!payload_only)
          packet_length += PREAMBLE_LOW_RATE_CLASS;

        data_rate = 1.0; // Lo Rate 9.6 kbps: 1.2 bytes/tick
        break;
    case PHY_CLASS_NORMAL_RATE:
        if(!payload_only)
          packet_length += PREAMBLE_NORMAL_RATE_CLASS;

        data_rate = 6.0; // Normal Rate 55.555 kbps: 6.94 bytes/tick
        break;
    case PHY_CLASS_HI_RATE:
        if(!payload_only)
          packet_length += PREAMBLE_HI_RATE_CLASS;

        data_rate = 20.0; // High rate 166.667 kbps: 20.83 byte/tick
        break;
    }

    // TODO Add the power ramp-up/ramp-down symbols in the packet length?

    return ceil(packet_length / data_rate) + 1;
}
