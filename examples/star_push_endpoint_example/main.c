/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
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

#include <d7aoss.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <framework/log.h>
#include <framework/timer.h>

#define SEND_INTERVAL_MS 2000
#define TX_EIRP 10

// Macro which can be removed in production environment
#define USE_LEDS

static uint8_t tx = 0;
static uint16_t counter = 0;
static volatile bool add_tx_event = true;

uint8_t buffer[32];
static uint8_t data[2];
static volatile uint8_t dataLength = 0;

static uint8_t send_channel[2] = {0x04, 0x00};

//static D7AQP_Command command;
static ALP_File_Data_Template data_template;
static ALP_Template alp_template;

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

		alp_create_structure_for_tx(ALP_REC_FLG_TYPE_UNSOLICITED, 0, 1, &alp_template);
		trans_tx_query(NULL, 0xFF, (uint8_t*) &send_channel, TX_EIRP);
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
	// Currently we address the Transport Layer, this should go to an upper layer once it is working.
	d7aoss_init(buffer, 128, buffer, 128);

	trans_set_tx_callback(&tx_callback);
	// The initial Tca for the CSMA-CA in
	dll_set_initial_t_ca(200);

	event.next_event = SEND_INTERVAL_MS;
	event.f = &start_tx;

	log_print_string("endpoint started");

	// No response (no acknowledgement)
	//command.command_code = D7AQP_COMMAND_CODE_EXTENSION | D7AQP_COMMAND_TYPE_NA2P_REQUEST | D7AQP_OPCODE_ANNOUNCEMENT_FILE;
	//command.command_extension = D7AQP_COMMAND_EXTENSION_NORESPONSE;
	//command.dialog_template = NULL;

	// Waiting for response (acknowledgement)
//	command.command_code = D7AQP_COMMAND_TYPE_NA2P_REQUEST | D7AQP_OPCODE_ANNOUNCEMENT_FILE;
//
//	D7AQP_Dialog_Template dialog_template;
//	dialog_template.response_timeout = 200;
//	dialog_template.response_channel_list_lenght = 0; // means same as send channel
//
//	command.dialog_template = &dialog_template;
//
//	D7AQP_Single_File_Return_Template file_template;
//	file_template.return_file_id = 0;
//	file_template.file_offset = 0;
//	file_template.isfb_total_length = 2;
//	file_template.file_data = data;
//
//	command.command_data = &file_template;

	alp_template.op = ALP_OP_RESP_DATA;
	alp_template.data = (uint8_t*) &data_template;

	data_template.file_id = 0;
	data_template.start_byte_offset = 0;
	data_template.bytes_accessing = 2;
	data_template.data = data;

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
