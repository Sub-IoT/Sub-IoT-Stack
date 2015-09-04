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

#include "hwleds.h"
#include "scheduler.h"
#include "timer.h"
#include "log.h"
#include <assert.h>
#include "platform.h"

#include "d7ap_stack.h"
#include "fs.h"
#include "dll.h"
#include "packet_queue.h"


void send_message()
{
	led_on(0);
	timer_post_task_delay(&send_message, TIMER_TICKS_PER_SEC);

	uint32_t time = timer_get_counter_value();
	fs_write_file(0x40, 0, (uint8_t*)&time, 2);
	log_print_string("sending message");
}


void dll_packet_transmitted()
{
	led_off(0);
	log_print_string("message send");
}

void start_foreground_scan()
{
    // TODO we start FG scan manually now, later it should be started by access profile automatically
    dll_start_foreground_scan();
}

void dll_packet_received()
{
	packet_t* packet = packet_queue_get_received_packet();
	if (packet)
	{
		//packet->hw_radio_packet.data[15];
		//packet->hw_radio_packet.data[16];
		led_toggle(1);
	}
	packet_queue_free_packet(packet);
}


void bootstrap()
{
	log_print_string("Device booted at time: %d\n", timer_get_counter_value());

	d7ap_stack_init();

    sched_register_task(&send_message);
    sched_register_task(&start_foreground_scan);
    //sched_post_task(&start_foreground_scan);
    dll_register_rx_callback(&dll_packet_received);
    dll_register_tx_callback(&dll_packet_transmitted);

    timer_post_task_delay(&send_message, TIMER_TICKS_PER_SEC);

}

