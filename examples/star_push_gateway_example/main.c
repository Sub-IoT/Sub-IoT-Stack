/*

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
 * Contributors:
 *     maarten.weyn@uantwerpen.be
 *
 * 	Example code for Star topology, push model
 * 	This is the Gateway example
 *
 * 	add the link to d7aoss library in de lnk_*.cmd file, e.g. -l "../../../d7aoss/Debug/d7aoss.lib"
 * 	Make sure to select the correct platform in d7aoss/hal/cc430/platforms.platform.h
 * 	If your platform is not present, you can add a header file in platforms and commit it to the repository.
 * 	Exclude the stub directories in d7aoss from the build when building for a device.
 */


#include <string.h>

#include <trans/trans.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <hal/uart.h>
#include <framework/log.h>
#include <framework/timer.h>

#define RECEIVE_CHANNEL 0x10
#define TX_EIRP 10
#define USE_LEDS

// event to create a led blink
static timer_event dim_led_event;
static bool start_channel_scan = false;

static D7AQP_Command command;

void blink_led()
{
	led_on(1);

	timer_add_event(&dim_led_event);
}

void dim_led()
{
	led_off(1);
}

void start_rx()
{
	start_channel_scan = false;
	trans_rx_query_start(0xFF, RECEIVE_CHANNEL);
}

void rx_callback(Trans_Rx_Query_Result* rx_res)
{
	system_watchdog_timer_reset();

	blink_led();

	dll_foreground_frame_t* frame = (dll_foreground_frame_t*) (rx_res->nwl_rx_res->dll_rx_res->frame);
	log_print_string("Received Query from :%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x;",
					frame->address_ctl->source_id[0] >> 4, frame->address_ctl->source_id[0] & 0x0F,
					frame->address_ctl->source_id[1] >> 4, frame->address_ctl->source_id[1] & 0x0F,
					frame->address_ctl->source_id[2] >> 4, frame->address_ctl->source_id[2] & 0x0F,
					frame->address_ctl->source_id[3] >> 4, frame->address_ctl->source_id[3] & 0x0F,
					frame->address_ctl->source_id[4] >> 4, frame->address_ctl->source_id[4] & 0x0F,
					frame->address_ctl->source_id[5] >> 4, frame->address_ctl->source_id[5] & 0x0F,
					frame->address_ctl->source_id[6] >> 4, frame->address_ctl->source_id[6] & 0x0F,
					frame->address_ctl->source_id[7] >> 4, frame->address_ctl->source_id[7] & 0x0F);
	log_print_string("RSS: %d dBm", rx_res->nwl_rx_res->dll_rx_res->rssi);
	log_print_string("Netto Link: %d dBm", rx_res->nwl_rx_res->dll_rx_res->rssi  - frame->frame_header.tx_eirp);

	switch (rx_res->d7aqp_command.command_code & 0x0F)
	{
		case D7AQP_OPCODE_ANNOUNCEMENT_FILE:
		{
			D7AQP_Single_File_Return_Template* sfr_tmpl = (D7AQP_Single_File_Return_Template*) rx_res->d7aqp_command.command_data;
			log_print_string("D7AQP File Announcement received");
			log_print_string(" - file 0x%x starting from byte %d", sfr_tmpl->return_file_id, sfr_tmpl->file_offset);
			log_print_data(sfr_tmpl->file_data, sfr_tmpl->isfb_total_length - sfr_tmpl->file_offset);
		}
	}

	if (rx_res->d7aqp_command.command_extension & D7AQP_COMMAND_EXTENSION_NORESPONSE)
	{
		// Restart channel scanning
		start_channel_scan = true;
	} else {
		// send ack
		// todo: put outside interrupt
		// todo: use dialog template

		command.command_code = D7AQP_COMMAND_CODE_EXTENSION | D7AQP_COMMAND_TYPE_RESPONSE | D7AQP_OPCODE_ANNOUNCEMENT_FILE;
		command.command_extension = D7AQP_COMMAND_EXTENSION_NORESPONSE;
		command.dialog_template = NULL;
		command.command_data = NULL;

		led_on(3);
		trans_tx_query(&command, 0xFF, RECEIVE_CHANNEL, TX_EIRP);
	}

}


void tx_callback(Trans_Tx_Result result)
{
	if(result == TransPacketSent)
	{
		#ifdef USE_LEDS
		led_off(3);
		#endif
		log_print_string("ACK SEND");
	}
	else
	{
		#ifdef USE_LEDS
		led_toggle(2);
		#endif
		log_print_string("TX ACK CCA FAIL");
	}

	// Restart channel scanning
	start_channel_scan = true;
}

int main(void) {
	// Initialize the OSS-7 Stack
	system_init();

	// Currently we address the Transport Layer for RX, this should go to an upper layer once it is working.
	trans_init();
	trans_set_query_rx_callback(&rx_callback);
	trans_set_tx_callback(&tx_callback);
	// The initial Tca for the CSMA-CA in
	trans_set_initial_t_ca(200);

	start_channel_scan = true;

	log_print_string("gateway started");

	// Log the device id
	log_print_data(device_id, 8);

	// configure blinking led event
	dim_led_event.next_event = 50;
	dim_led_event.f = &dim_led;

	system_watchdog_init(0x0020, 0x03);
	system_watchdog_timer_start();

	blink_led();

	while(1)
	{
		if (start_channel_scan)
		{
			start_rx();
		}

		// Don't know why but system reboots when LPM > 1 since ACLK is uses for UART
		system_lowpower_mode(0,1);
	}
}
