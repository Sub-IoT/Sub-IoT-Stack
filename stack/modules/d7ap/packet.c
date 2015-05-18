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
#include "crc.h"

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

    // TODO network protocol footer

    // add CRC
    uint16_t crc = __builtin_bswap16(crc_calculate(packet->hw_radio_packet.data, packet->hw_radio_packet.length - 2));
    memcpy(data_ptr, &crc, 2);
}
