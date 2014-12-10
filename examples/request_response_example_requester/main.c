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
 * 	Make sure to select the correct platform in d7aoss.h
 * 	If your platform is not present, you can add a header file in platforms and commit it to the repository.
 * 	Exclude the stub directories in d7aoss from the build when building for a device.
 *  Create the apporpriate file system settings for the FLASH system:
 *
 * 	Add following sections to the SECTIONS in .cmd linker file to use the filesystem
 *		.fs_fileinfo_bitmap : 	{} > FLASH_FS1
 *  	.fs_fileinfo: 			{} > FLASH_FS1
 *		.fs_files	: 			{} > FLASH_FS2
 *
 *	Add FLASH_FS_FI and FLASH_FS_FILES to the MEMORY section
 *  eg.
 *  	FLASH_FS1               : origin = 0xC000, length = 0x0200 // The file headers
 *	    FLASH_FS2               : origin = 0xC200, length = 0x0400 // The file contents
 */


#include <string.h>

#include <d7aoss.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <framework/log.h>
#include <framework/timer.h>
#include <msp430.h>


#define SEND_INTERVAL_MS 5000
#define TX_EIRP 10
#define CLOCKS_PER_1us	4

// Macro which can be removed in production environment
#define USE_LEDS

static uint8_t send_channel[2] = {0x04, 0x00};
static uint8_t tx = 0;
static uint8_t counter = 0;
static volatile bool add_tx_event = true;
static volatile bool wait_for_response = false;

uint8_t tx_buffer[128];
uint8_t rx_buffer[128];

static volatile uint8_t dataLength = 0;


void start_tx()
{
	ALP_File_Data_Template data_template;
	ALP_Template alp_template;

	if (!tx)
	{
		// Kicks the watchdog timer
		system_watchdog_timer_reset();

		tx = 1;

		#ifdef USE_LEDS
		led_on(3);
		#endif

		log_print_string("TX Request");

		alp_template.op = ALP_OP_READ_DATA;
		alp_template.data = (uint8_t*) &data_template;

		data_template.file_id = 32;
		data_template.start_byte_offset = 0;
		data_template.bytes_accessing = 6;
		data_template.data = NULL;

		alp_create_structure_for_tx(ALP_REC_FLG_TYPE_COMMAND_RESPONSE, counter, 1, &alp_template);
		trans_tx_query(NULL, 0xFF, send_channel, TX_EIRP, true);

	}

	add_tx_event = true;
}

void rx_callback(Trans_Rx_Alp_Result* rx_res)
{
	system_watchdog_timer_reset();

	led_on(1);


	log_print_string("Received Query");

	if (rx_res != NULL)
	{
		uint8_t alp_op = rx_res->alp_record.alp_templates[0];
		uint8_t* alp_data = &rx_res->alp_record.alp_templates[1];

		if (alp_op == ALP_OP_RESP_DATA)
		{
			volatile ALP_File_Data_Template data;
			data.file_id  = alp_data[0];
			data.start_byte_offset =  MERGEUINT16(alp_data[1], alp_data[2]);
			data.bytes_accessing =  MERGEUINT16(alp_data[3], alp_data[4]);
			data.data = &alp_data[5];

			log_print_string("D7AQP File Response Data");
			log_print_string(" - file 0x%x starting from byte %d", data.file_id, data.start_byte_offset);
			log_print_data(data.data, data.bytes_accessing - data.start_byte_offset);
		} else {
			log_print_string("Unexpected query received");
		}
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
	d7aoss_init(tx_buffer, 128, rx_buffer, 128);


	trans_set_tx_callback(&tx_callback);
    // The initial Tca for the CSMA-CA in
	dll_set_initial_t_ca(200);

	trans_set_alp_rx_callback(&rx_callback);

	event.next_event = SEND_INTERVAL_MS;
	event.f = &start_tx;

	log_print_string("Requester started");

	timer_add_event(&event);

	// Log the device id
	log_print_data(device_id, 8);

	while(1)
	{
		if (wait_for_response)
		{
			wait_for_response = false;
			log_print_string("Starting RX");
			trans_rx_query_start(0xFF, send_channel);
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

