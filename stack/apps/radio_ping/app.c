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

#include "types.h"
#include "scheduler.h"
#include "timer.h"
#include "hwradio.h"
#include "hwsystem.h"
#include "log.h"
#include "ng.h"
#include "random.h"
#include "types.h"
#include "debug.h"


typedef struct __attribute__((__packed__))
{
	hw_radio_packet_t radio_packet;
	uint16_t src_node;
	uint16_t dst_node;
	uint16_t counter;
} packet_struct_t;


uint8_t NGDEF(rx_buffer)[HW_PACKET_BUF_SIZE(255)];
bool NGDEF(rx_buf_used);
packet_struct_t NGDEF(tx_buffer);

static hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
        .channel_header.ch_freq_band = PHY_BAND_868,
        .center_freq_index = 0
    },
    .syncword_class = PHY_SYNCWORD_CLASS0
};

static hw_radio_packet_t* p_alloc(uint8_t length)
{
	if(NG(rx_buf_used))
		return 0x0;
	else
	{
		NG(rx_buf_used) = 1;
		return (hw_radio_packet_t*)(&NG(rx_buffer));
	}
}

static void p_free(hw_radio_packet_t* packet)
{
	assert((void*)packet == (void*)&NG(rx_buffer));
	NG(rx_buf_used) = false;
}

static void rssi_valid(int16_t rssi)
{
	log_print_string("Rssi valid at time %u", timer_get_counter_value());
}

void send_packet();

//typedef struct
//{
//	int16_t rssi;
//	bool is_rx;
//	bool tx_busy;
//	bool rx_busy;
//	bool is_sleep;
//} status_t;
//
//
//status_t NGDEF(status);
//
//void poll_status()
//{
//	status_t cur_status = {.is_rx = hw_radio_is_rx(),
//						   .rx_busy = hw_radio_rx_busy(),
//						   .tx_busy = hw_radio_tx_busy(),
//						   .is_sleep = hw_radio_is_idle(),
//						   .rssi = hw_radio_get_rssi()};
//
//	if(hw_get_unique_id() == 1 && (cur_status.is_rx != NG(status).is_rx
//			|| cur_status.tx_busy != NG(status).tx_busy
//			|| cur_status.rx_busy != NG(status).rx_busy
//			|| cur_status.is_sleep != NG(status).is_sleep
//			|| ((cur_status.rssi == HW_RSSI_INVALID) != (NG(status).rssi == HW_RSSI_INVALID))))
//	{
//		NG(status) = cur_status;
//		log_print_string("sleep: %u, rx: %u, tx_busy: %u, rx_busy: %u, rssi: %d, Time: %u", cur_status.is_sleep, cur_status.is_rx, cur_status.tx_busy, cur_status.rx_busy, cur_status.rssi, timer_get_counter_value());
//	}
//	timer_post_task_delay(poll_status, 10);
//}

static void tx_callback(hw_radio_packet_t* tx_packet)
{
	log_print_string("Sent packet with counter %u", NG(tx_buffer).counter);
	timer_post_task_delay(send_packet, TIMER_TICKS_PER_SEC + (get_rnd() %TIMER_TICKS_PER_SEC));
}

static void rx_callback(hw_radio_packet_t* tx_packet)
{
	if(tx_packet->length < sizeof(packet_struct_t) - sizeof(hw_radio_packet_t))
		log_print_string("Got an invalid packet at time %d: packet was too short: %d < %d", timer_get_counter_value(), tx_packet->length, sizeof(packet_struct_t));
	else
	{
		packet_struct_t* packet = (packet_struct_t*)tx_packet;
		if(packet->radio_packet.length != sizeof(packet_struct_t) - sizeof(hw_radio_packet_t))
			log_print_string("Got an invalid packet at time %d: packet length didn't match", timer_get_counter_value());
		log_print_string("Got packet from %u to %u, seq: %u at time %u", packet->src_node, packet->dst_node, packet->counter, timer_get_counter_value());
	}
	p_free(tx_packet);
}

void send_packet()
{
	hw_radio_send_packet((hw_radio_packet_t*)(&NG(tx_buffer)), tx_callback);
	log_print_string("Sending packet with counter %u", NG(tx_buffer).counter);
	NG(tx_buffer).counter++;
	timer_post_task_delay(send_packet, TIMER_TICKS_PER_SEC + (get_rnd() %TIMER_TICKS_PER_SEC));
}

void bootstrap()
{
    log_print_string("Device booted at time: %d\n", timer_get_counter_value());

    hw_radio_init(p_alloc, p_free);
    hw_radio_set_rx(&rx_cfg, rx_callback, rssi_valid);


    NG(tx_buffer).radio_packet.length = sizeof(packet_struct_t) - sizeof(hw_radio_packet_t);
    NG(tx_buffer).radio_packet.tx_meta.tx_cfg.channel_id = rx_cfg.channel_id;
    NG(tx_buffer).radio_packet.tx_meta.tx_cfg.syncword_class = rx_cfg.syncword_class;
    NG(tx_buffer).radio_packet.tx_meta.tx_cfg.eirp = 0;

	NG(tx_buffer).src_node = hw_get_unique_id();
    NG(tx_buffer).dst_node = 0xFFFF;
    NG(tx_buffer).counter = 0;


    sched_register_task(&send_packet);
    timer_post_task_delay(&send_packet, TIMER_TICKS_PER_SEC + (get_rnd() %TIMER_TICKS_PER_SEC));

//    NG(status).is_rx = false;
//    NG(status).is_sleep = true;
//    NG(status).tx_busy = false;
//    NG(status).rx_busy = false;
//    sched_register_task(&poll_status);
//    sched_post_task(poll_status);

}


