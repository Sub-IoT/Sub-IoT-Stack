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
 */

#include "packet.h"
#include "packet_queue.h"
#include "crc.h"
#include "log.h"
#include "d7asp.h"
#include "fec.h"
#include "phy.h"
#include "MODULE_D7AP_defs.h"

#include "debug.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_FWK_LOG_ENABLED)
#define DPRINT_FWK(...) log_print_stack_string(LOG_STACK_FWK, __VA_ARGS__)
#else
#define DPRINT_FWK(...)
#endif

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_DLL_LOG_ENABLED)
#define DPRINT_DLL(...) log_print_stack_string(LOG_STACK_DLL, __VA_ARGS__)
#define DPRINT_DATA_DLL(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT_DLL(...)
#define DPRINT_DATA_DLL(...)
#endif

#ifdef HAL_RADIO_USE_HW_CRC
static bool has_hardware_crc = true;
#else
static bool has_hardware_crc = false;
#endif

void packet_init(packet_t* packet)
{
    memset(packet, 0x00, sizeof(packet_t));
}

void packet_assemble(packet_t* packet)
{
    uint8_t* data_ptr = packet->hw_radio_packet.data + 1; // skip length field for now, we fill this later

    data_ptr += dll_assemble_packet_header(packet, data_ptr);

    data_ptr += d7anp_assemble_packet_header(packet, data_ptr);

#if defined(MODULE_D7AP_NLS_ENABLED)
    uint8_t* nwl_payload = data_ptr;
#endif

    data_ptr += d7atp_assemble_packet_header(packet, data_ptr);

    // add payload
    memcpy(data_ptr, packet->payload, packet->payload_length); data_ptr += packet->payload_length;

#if defined(MODULE_D7AP_NLS_ENABLED)
    /* Encrypt/authenticate nwl_payload if needed */
    if (packet->d7anp_ctrl.nls_method)
        data_ptr += d7anp_secure_payload(packet, nwl_payload, data_ptr - nwl_payload);
#endif

    packet->hw_radio_packet.length = data_ptr - packet->hw_radio_packet.data + 2; // exclude the CRC bytes
    packet->hw_radio_packet.data[0] = packet->hw_radio_packet.length - 1; // exclude the length byte

    // TODO network protocol footer

    // add CRC - SW CRC when using FEC
    if (!has_hardware_crc ||
              packet->phy_config.tx.channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
    {
        uint16_t crc = __builtin_bswap16(crc_calculate(packet->hw_radio_packet.data, packet->hw_radio_packet.length - 2));
        memcpy(data_ptr, &crc, 2);
    }

}

void packet_disassemble(packet_t* packet)
{

    if (packet->hw_radio_packet.rx_meta.crc_status == HW_CRC_UNAVAILABLE)
    {
        uint16_t crc;
        crc = __builtin_bswap16(crc_calculate(packet->hw_radio_packet.data, packet->hw_radio_packet.length - 2));

        if(memcmp(&crc, packet->hw_radio_packet.data + packet->hw_radio_packet.length - 2, 2) != 0)
        {
            log_print_error_string("CRC invalid");
            DPRINT_DLL("Packet: len %d", packet->hw_radio_packet.length);
            DPRINT_DATA_DLL(packet->hw_radio_packet.data, packet->hw_radio_packet.length);
            goto cleanup;
        }
    }
    else if (packet->hw_radio_packet.rx_meta.crc_status == HW_CRC_INVALID)
    {
        DPRINT_DLL("CRC invalid");
        goto cleanup;
    }

    uint8_t data_idx;
    if (packet->type != BACKGROUND_ADV)
        data_idx = 1;
    else
        data_idx = 0;

    if(!dll_disassemble_packet_header(packet, &data_idx))
        goto cleanup;

    if (packet->type != BACKGROUND_ADV)
    {
        if(!d7anp_disassemble_packet_header(packet, &data_idx))
            goto cleanup;

        if(!d7atp_disassemble_packet_header(packet, &data_idx))
            goto cleanup;

        // extract payload
        if(packet->hw_radio_packet.length < (data_idx + 2))
        {
            goto cleanup;
        }
        packet->payload_length = packet->hw_radio_packet.length - data_idx - 2; // exclude the headers CRC bytes // TODO exclude footers
        if(packet->payload_length > MODULE_D7AP_PAYLOAD_SIZE)
        {
            goto cleanup;
        }
        memcpy(packet->payload, packet->hw_radio_packet.data + data_idx, packet->payload_length);
    }
    else
    {
        // extract ETA for background frames
        uint16_t eta;
        if(packet->hw_radio_packet.length < (data_idx + 2))
        {
            goto cleanup;
        }
        packet->payload_length = packet->hw_radio_packet.length - data_idx - 2; // exclude the headers CRC bytes // TODO exclude footers
        assert(packet->payload_length == sizeof(uint16_t));

        memcpy(&eta, packet->hw_radio_packet.data + data_idx, packet->payload_length);
        packet->ETA = __builtin_bswap16(eta);
    }
    // TODO footers


    DPRINT_FWK("Done disassembling packet");

    d7anp_process_received_packet(packet);

    return;

    cleanup:
        log_print_error_string("Skipping packet");
        packet_queue_free_packet(packet);
        return;
}
