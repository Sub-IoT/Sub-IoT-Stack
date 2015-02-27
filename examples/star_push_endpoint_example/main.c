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
 * 	This is the endpoint example
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
#include <framework/log.h>
#include <framework/timer.h>

#define SEND_INTERVAL_MS 2000
#define SEND_CHANNEL 0x10
#define TX_EIRP 10

// Macro which can be removed in production environment
#define USE_LEDS

static uint8_t tx = 0;
static uint16_t counter = 0;
static volatile bool add_tx_event = true;

static uint8_t data[32];
static volatile uint8_t dataLength = 0;

static D7AQP_Command command;

void start_tx()
{
	if (!tx)
	{
		// Kicks the watchdog timer
		system_watchdog_timer_reset();

		tx = 1;

		#ifdef USE_LEDS
		led_on(3);
		#endif

		log_print_string("TX...");

		data[0] = counter >> 8;
		data[1] = counter & 0xFF;

		trans_tx_query(&command, 0xFF, SEND_CHANNEL, TX_EIRP);
	}
	add_tx_event = true;
}

void tx_callback(Trans_Tx_Result result)
{
	counter++;

	if(result == TransPacketSent)
	{
		#ifdef USE_LEDS
		led_off(3);
		#endif
		log_print_string("TX OK");
	}
	else
	{
		#ifdef USE_LEDS
		led_toggle(1);
		#endif
		log_print_string("TX CCA FAIL");
	}

	tx = 0;
}

int main(void) {
	timer_event event;

	// Initialize the OSS-7 Stack
	system_init();

	// Currently we address the Transport Layer, this should go to an upper layer once it is working.
	trans_init();
	trans_set_tx_callback(&tx_callback);
	// The initial Tca for the CSMA-CA in
	trans_set_initial_t_ca(200);

	event.next_event = SEND_INTERVAL_MS;
	event.f = &start_tx;

	log_print_string("endpoint started");

	// No response (no acknowledgement)
	command.command_code = D7AQP_COMMAND_CODE_EXTENSION | D7AQP_COMMAND_TYPE_NA2P_REQUEST | D7AQP_OPCODE_ANNOUNCEMENT_FILE;
	command.command_extension = D7AQP_COMMAND_EXTENSION_NORESPONSE;
	command.dialog_template = NULL;

	// Waiting for response (acknowledgement)
//	command.command_code = D7AQP_COMMAND_TYPE_NA2P_REQUEST | D7AQP_OPCODE_ANNOUNCEMENT_FILE;
//
//	D7AQP_Dialog_Template dialog_template;
//	dialog_template.response_timeout = 200;
//	dialog_template.response_channel_list_lenght = 0; // means same as send channel
//
//	command.dialog_template = &dialog_template;

	D7AQP_Single_File_Return_Template file_template;
	file_template.return_file_id = 0;
	file_template.file_offset = 0;
	file_template.isfb_total_length = 2;
	file_template.file_data = data;

	command.command_data = &file_template;

	timer_add_event(&event);

	// Log the device id
	log_print_data(device_id, 8);

	system_watchdog_init(0x0020, 0x03);
	system_watchdog_timer_start();

	while(1)
	{
		if (add_tx_event)
		{
			add_tx_event = false;
			timer_add_event(&event);
		}

		system_lowpower_mode(3,1);
	}
}
