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

#include "packet.h"
#include "packet_queue.h"
#include "crc.h"
#include "log.h"
#include "d7asp.h"
#include "fec.h"
#include "MODULE_D7AP_defs.h"

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
    // TODO
}

void packet_assemble(packet_t* packet)
{
    uint8_t* data_ptr = packet->hw_radio_packet.data + 1; // skip length field for now, we fill this later

    data_ptr += dll_assemble_packet_header(packet, data_ptr);

    data_ptr += d7anp_assemble_packet_header(packet, data_ptr);

    data_ptr += d7atp_assemble_packet_header(packet, data_ptr);

    // add payload
    memcpy(data_ptr, packet->payload, packet->payload_length); data_ptr += packet->payload_length;
    packet->hw_radio_packet.length = data_ptr - packet->hw_radio_packet.data - 1 + 2; // exclude the length byte and add CRC bytes
    packet->hw_radio_packet.data[0] = packet->hw_radio_packet.length;

    // TODO network protocol footer

    // add CRC - SW CRC when using FEC
    if (!has_hardware_crc || packet->hw_radio_packet.tx_meta.tx_cfg.channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
    {
    	DPRINT_DATA_DLL(packet->hw_radio_packet.data, packet->hw_radio_packet.length + 1); // TODO tmp
    	uint16_t crc = __builtin_bswap16(crc_calculate(packet->hw_radio_packet.data, packet->hw_radio_packet.length + 1 - 2));
    	memcpy(data_ptr, &crc, 2);
    	DPRINT_DATA_DLL(packet->hw_radio_packet.data, packet->hw_radio_packet.length + 1); // TODO tmp
    }

}

void packet_disassemble(packet_t* packet)
{
	DPRINT_DATA_DLL(packet->hw_radio_packet.data, packet->hw_radio_packet.length + 1); // TODO tmp

    if (packet->hw_radio_packet.rx_meta.crc_status == HW_CRC_UNAVAILABLE)
    {
        uint16_t crc = __builtin_bswap16(crc_calculate(packet->hw_radio_packet.data, packet->hw_radio_packet.length + 1 - 2));
        if(memcmp(&crc, packet->hw_radio_packet.data + packet->hw_radio_packet.length + 1 - 2, 2) != 0)
        {
            DPRINT_DLL("CRC invalid");
            DPRINT_DATA_DLL((uint8_t*)&crc, 2);
            goto cleanup;
        }
    }
    else if (packet->hw_radio_packet.rx_meta.crc_status == HW_CRC_INVALID)
    {
        DPRINT_DLL("CRC invalid");
        goto cleanup;
    }

    uint8_t data_idx = 1;

    if(!dll_disassemble_packet_header(packet, &data_idx))
        goto cleanup;

    // TODO assuming D7ANP for now
    if(!d7anp_disassemble_packet_header(packet, &data_idx))
        goto cleanup;

    if(!d7atp_disassemble_packet_header(packet, &data_idx))
        goto cleanup;

    // TODO footers

    // extract payload
    packet->payload_length = packet->hw_radio_packet.length + 1 - data_idx - 2; // exclude the headers CRC bytes // TODO exclude footers
    memcpy(packet->payload, packet->hw_radio_packet.data + data_idx, packet->payload_length);

    DPRINT_FWK("Done disassembling packet");

    d7atp_process_received_packet(packet);

    return;

    cleanup:
        DPRINT_FWK("Skipping packet");
        packet_queue_free_packet(packet);
        return;
}
