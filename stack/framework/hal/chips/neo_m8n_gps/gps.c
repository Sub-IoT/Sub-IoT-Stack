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

/*! \file gps.c
 *
 *  \author Liam Wickins <liamw9534@gmail.com>
 *
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "hwuart.h"
#include "platform_defs.h"
#include "scheduler.h"
#include "timer.h"
#include "fifo.h"
#include "gps.h"
#include "ubx.h"
#include "log.h"

#define GPS_INTERVAL_SEC 		(1 * TIMER_TICKS_PER_SEC)
#define GPS_BUFFER_SIZE			256


// UBX messages are received into a FIFO area of memory and parsed periodically
// at a rate of GPS_INTERVAL_SEC.  The parsing function will extract complete
// UBX packets once they have been fully received into the FIFO.
static uart_handle_t* uart;
static gps_callback_t gps_callback = NULL;
static bool is_gps_enabled = false;
static bool is_gps_fixed = false;
static fifo_t gps_fifo;
static uint8_t gps_fifo_buffer[GPS_BUFFER_SIZE];
static uint32_t ttff;


void ubx_checksum(UBX_Packet_t *packet, uint8_t ck[2])
{
    ck[0] = ck[1] = 0;
    uint8_t *buffer = &packet->msgClass;

    /* Taken directly from Section 29 UBX Checksum of the u-blox8
     * receiver specification
     */
    for (unsigned int i = 0; i < packet->msgLength + 4; i++)
    {
        ck[0] = ck[0] + buffer[i];
        ck[1] = ck[1] + ck[0];
    }
}

void ubx_set_checksum(UBX_Packet_t *packet)
{
    uint8_t ck[2];

    ubx_checksum(packet, ck);
    packet->payloadAndCrc[packet->msgLength]   = ck[0];
    packet->payloadAndCrc[packet->msgLength+1] = ck[1];
}

void ubx_send(UBX_Packet_t *packet)
{
	unsigned int total_length = UBX_HEADER_AND_CRC_LENGTH + packet->msgLength;
	uart_send_bytes(uart, (uint8_t *)packet, total_length);
}

int parse_gps_fifo(UBX_Packet_t *packet)
{
	uint8_t byte;
	uint32_t occupancy;
	int idx;

	occupancy = fifo_get_size(&gps_fifo);

	/* Check for minimum allowed message size */
    if (occupancy < UBX_HEADER_AND_CRC_LENGTH)
        return -1;

    /* Find SYNC1 byte */
    for (idx = 0; idx <= (occupancy - UBX_HEADER_AND_CRC_LENGTH); idx++)
    {
    	fifo_peek(&gps_fifo, &byte, idx, 1);
        if (UBX_PACKET_SYNC_CHAR1 == byte)
            goto ubx_sync_start;
    }

    /* No SYNC1 so flush checked bytes so far... */
    fifo_skip(&gps_fifo, idx);
    return -1;

ubx_sync_start:
    /* Check SYNC2 is present */
	fifo_peek(&gps_fifo, &byte, idx + 1, 1);
    if (UBX_PACKET_SYNC_CHAR2 != byte)
    {
        fifo_skip(&gps_fifo, idx + 2);
        return -1;  /* Invalid SYNC2 */
    }

    /* Extract length field and check it is received fully */
    uint16_t payload_length;
    fifo_peek(&gps_fifo, &byte, idx + 4, 1);
    payload_length = byte;
    fifo_peek(&gps_fifo, &byte, idx + 5, 1);
    payload_length |= (uint16_t)byte << 8;
    uint16_t total_length = payload_length + UBX_HEADER_AND_CRC_LENGTH;
    if (total_length > GPS_BUFFER_SIZE)
    {
        /* Message is too big to store so throw it away */
    	fifo_clear(&gps_fifo);
        return -1;
    }

    /* Check message is fully received */
    if (total_length > occupancy)
        return -1;

    /* Copy message into packet structure */
    fifo_peek(&gps_fifo, (uint8_t *)packet, idx, total_length);
    fifo_skip(&gps_fifo, idx + total_length);

    return 0;
}


void gps_process()
{
	if (is_gps_enabled)
	{
		UBX_Packet_t packet;
		int ret;
		do
		{
			ret = parse_gps_fifo(&packet);
			if (ret == 0)
			{
				//printf("UBX=%02x-%02x ITOW=%u\n", packet.msgClass, packet.msgId, packet.UBX_NAV_STATUS.iTOW);
				if (UBX_IS_MSG(packet, UBX_MSG_CLASS_NAV, UBX_MSG_ID_NAV_STATUS))
				{
					is_gps_fixed = packet.UBX_NAV_STATUS.gpsFix;
					ttff = packet.UBX_NAV_STATUS.ttff;
				}
				if (UBX_IS_MSG(packet, UBX_MSG_CLASS_NAV, UBX_MSG_ID_NAV_POSLLH) && gps_callback)
				{
					gps_event_t evt;
					evt.id = GPS_EVENT_POS;
					evt.pos.is_fixed = is_gps_fixed;
					evt.pos.itow = packet.UBX_NAV_POSLLH.iTOW;
					evt.pos.ttff = ttff;
					evt.pos.height = packet.UBX_NAV_POSLLH.height;
					evt.pos.longitude = packet.UBX_NAV_POSLLH.lon;
					evt.pos.latitude = packet.UBX_NAV_POSLLH.lat;
					gps_callback(&evt);
				}
			}
		} while (ret == 0);
		timer_post_task_delay(&gps_process, GPS_INTERVAL_SEC);
	}
}


void gps_uart_rx_cb(uint8_t byte)
{
	fifo_put_byte(&gps_fifo, byte);
}

void gps_init(gps_callback_t cb)
{
	uart = uart_init(PLATFORM_GPS_UART, PLATFORM_GPS_BAUDRATE, 0);
	assert(uart_enable(uart));
	uart_set_rx_interrupt_callback(uart, gps_uart_rx_cb);
	uart_rx_interrupt_enable(uart);
	gps_callback = cb;
	sched_register_task(&gps_process);
	fifo_init(&gps_fifo, gps_fifo_buffer, GPS_BUFFER_SIZE);
}

void gps_enable(void)
{
	if (is_gps_enabled)
		return;
	// Send a 4 byte message 00 00 00 00 to wakeup the module on its UART RX pin
	// This assumes the device was powered down using UBX_RXM_PMREQ with UART
	// wake-up source selected
	uint8_t buf[4] = { 0 };
	uart_send_bytes(uart, buf, 4);
	fifo_clear(&gps_fifo);
	is_gps_enabled = true;
	timer_post_task_delay(&gps_process, GPS_INTERVAL_SEC);
}

void gps_disable(void)
{
	if (!is_gps_enabled)
		return;
	// Send UBX_RX_PMREQ to put the device into low power state and allow it to
	// be woken up on its UART RX pin
	UBX_Packet_t packet;
	memset(&packet, 0, sizeof(packet));
	UBX_SET_PACKET_HEADER(packet, UBX_MSG_CLASS_RXM, UBX_MSG_ID_RXM_PMREQ, sizeof(packet.UBX_RXM_PMREQ));
	packet.UBX_RXM_PMREQ.version = 0;
	packet.UBX_RXM_PMREQ.flags = 0x6;
	packet.UBX_RXM_PMREQ.duration = 0;
	packet.UBX_RXM_PMREQ.wakeupSources = UBX_RXM_PMREQ_WAKEUP_SOURCE_UART;
	ubx_set_checksum(&packet);
	ubx_send(&packet);
	is_gps_enabled = false;
}
