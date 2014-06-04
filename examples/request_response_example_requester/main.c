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
 * 	Example of Request Response Dialog
 * 	This is the requester
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
#include <msp430.h>


#define SEND_INTERVAL_MS 5000
#define SEND_CHANNEL 0x10
#define TX_EIRP 10
#define USE_ACK 1
#define START_FILE_ID 0
#define END_FILE_ID 5

// Macro which can be removed in production environment
#define USE_LEDS

static uint8_t tx = 0;
static uint16_t counter = 0;
static volatile bool add_tx_event = true;
static volatile bool wait_for_response = false;

static volatile uint8_t dataLength = 0;

static D7AQP_Single_File_Call_Template file_template;

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

		file_template.return_file_id++;
		if (file_template.return_file_id > END_FILE_ID)
			file_template.return_file_id = START_FILE_ID;

		trans_tx_query(&command, 0xFF, SEND_CHANNEL, TX_EIRP);
	}
	add_tx_event = true;
}

void rx_callback(Trans_Rx_Query_Result* rx_res)
{
	system_watchdog_timer_reset();

	led_on(1);

	log_print_string("Received Query");

	if ((rx_res->d7aqp_command.command_code & 0x70 == 0) && (rx_res->d7aqp_command.command_code & 0x0F == D7AQP_OPCODE_COLLECTION_FILE_FILE & 0x0F))
	{
		log_print_string("Response Received");

		D7AQP_Single_File_Return_Template* sfr_tmpl = (D7AQP_Single_File_Return_Template*) rx_res->d7aqp_command.command_data;
		log_print_string("D7AQP File Announcement received");
		log_print_string(" - file 0x%x starting from byte %d", sfr_tmpl->return_file_id, sfr_tmpl->file_offset);
		log_print_data(sfr_tmpl->file_data, sfr_tmpl->isfb_total_length - sfr_tmpl->file_offset);

	} else {
		log_print_string("Unexpected query received");
	}

	led_off(1);
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

		wait_for_response = true;
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
	trans_set_query_rx_callback(&rx_callback);
	// The initial Tca for the CSMA-CA in
	trans_set_initial_t_ca(200);

	event.next_event = SEND_INTERVAL_MS;
	event.f = &start_tx;

	log_print_string("Requester started");

	command.command_code = D7AQP_COMMAND_TYPE_NA2P_REQUEST | D7AQP_OPCODE_COLLECTION_FILE_FILE;
	//
	D7AQP_Dialog_Template dialog_template;
	dialog_template.response_timeout = 200;
	dialog_template.response_channel_list_lenght = 0; // means same as send channel
	//
	command.dialog_template = &dialog_template;

	file_template.max_returned_bytes = 20;
	file_template.return_file_entry_offset = 0;
	file_template.return_file_id = END_FILE_ID;

	command.command_data = &file_template;

	timer_add_event(&event);

	// Log the device id
	log_print_data(device_id, 8);

	while(1)
	{
		if (wait_for_response)
		{
			trans_rx_query_start(0xFF, SEND_CHANNEL);
		}

		if (add_tx_event)
		{
			add_tx_event = false;
			timer_add_event(&event);
		}

		system_lowpower_mode(0,1);
	}
}


#pragma vector=ADC12_VECTOR,RTC_VECTOR,AES_VECTOR,COMP_B_VECTOR,DMA_VECTOR,PORT1_VECTOR,PORT2_VECTOR,SYSNMI_VECTOR,UNMI_VECTOR,USCI_A0_VECTOR,USCI_B0_VECTOR,WDT_VECTOR,TIMER0_A0_VECTOR,TIMER1_A1_VECTOR
__interrupt void ISR_trap(void)
{
  /* For debugging purposes, you can trap the CPU & code execution here with an
     infinite loop */
  //while (1);
	__no_operation();

  /* If a reset is preferred, in scenarios where you want to reset the entire system and
     restart the application from the beginning, use one of the following lines depending
     on your MSP430 device family, and make sure to comment out the while (1) line above */

  /* If you are using MSP430F5xx or MSP430F6xx devices, use the following line
     to trigger a software BOR.   */
  PMMCTL0 = PMMPW | PMMSWBOR;          // Apply PMM password and trigger SW BOR
}

