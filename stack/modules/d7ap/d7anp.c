/*! \file d7anp.c
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
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
 *
 */

#include "debug.h"
#include "d7anp.h"
#include "packet.h"
#include "fs.h"

void d7anp_tx_foreground_frame(packet_t* packet, bool should_include_origin_template, dae_access_profile_t* access_profile)
{
    packet->d7anp_ctrl.nls_enabled = false;
    packet->d7anp_ctrl.hop_enabled = false;
    packet->d7anp_ctrl.origin_access_id_present = should_include_origin_template;
    uint8_t vid[2];
    fs_read_vid(vid);
    if(memcmp(vid, (uint8_t[2]){0x00, 0x00},2) == 0)
        packet->d7anp_ctrl.origin_access_id_is_vid = false;
    else
        packet->d7anp_ctrl.origin_access_id_is_vid = true;

    packet->d7anp_ctrl.origin_access_class = packet->d7atp_addressee->addressee_ctrl_access_class; // TODO validate

    dll_tx_frame(packet, access_profile);
}

uint8_t d7anp_assemble_packet_header(packet_t *packet, uint8_t *data_ptr)
{
    assert(!packet->d7anp_ctrl.nls_enabled); // TODO NLS not yet supported
    assert(!packet->d7anp_ctrl.hop_enabled); // TODO hopping not yet supported

    uint8_t* d7anp_header_start = data_ptr;
    (*data_ptr) = packet->d7anp_timeout; data_ptr++;
    (*data_ptr) = packet->d7anp_ctrl.raw; data_ptr++;

    // TODO hopping ctrl
    // TODO intermediary access ID
    // TODO destination access ID

    if(packet->d7anp_ctrl.origin_access_id_present)
    {
        if(!packet->d7anp_ctrl.origin_access_id_is_vid)
        {
            fs_read_uid(data_ptr); data_ptr += 8;
        }
        else
        {
            fs_read_vid(data_ptr); data_ptr += 2;
        }
    }

    return data_ptr - d7anp_header_start;
}

bool d7anp_disassemble_packet_header(packet_t* packet, uint8_t* data_idx)
{
    packet->d7anp_timeout = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    packet->d7anp_ctrl.raw = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    assert(!packet->d7anp_ctrl.nls_enabled); // TODO NLS not yet supported
    assert(!packet->d7anp_ctrl.hop_enabled); // TODO hopping not yet supported

    // TODO hopping ctrl
    // TODO intermediary access ID
    // TODO destination access ID

    if(packet->d7anp_ctrl.origin_access_id_present)
    {
        uint8_t origin_access_id_size = packet->d7anp_ctrl.origin_access_id_is_vid? 2 : 8;
        memcpy(packet->origin_access_id, packet->hw_radio_packet.data + (*data_idx), origin_access_id_size); (*data_idx) += origin_access_id_size;
    }

    // TODO security

    return true;
}
