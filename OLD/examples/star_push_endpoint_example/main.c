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
 * 	Make sure to select the correct platform in d7aoss.h
 * 	If your platform is not present, you can add a header file in platforms and commit it to the repository.
 * 	Exclude the stub directories in d7aoss from the build when building for a device.
 *
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



#include <d7aoss.h>
#include <framework/log.h>
#include <framework/timer.h>
#include <hal/leds.h>

#define SEND_INTERVAL_MS 500
#define TX_EIRP 10

// Macro which can be removed in production environment
#define USE_LEDS

#ifdef UART
#define DPRINT(str) log_print(str)
#else
#define DPRINT(str)
#endif

static uint8_t tx = 0;
static uint16_t counter = 0;
static volatile bool add_tx_event = true;

uint8_t buffer[128];
static uint8_t data[4];
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

        DPRINT("TX...");

		data[0] = counter >> 8;
		data[1] = counter & 0xFF;

		alp_create_structure_for_tx(ALP_REC_FLG_TYPE_UNSOLICITED, 0, 1, &alp_template);
		trans_tx_query(NULL, 0xFF, send_channel, TX_EIRP, true);


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

        DPRINT("TX OK");
	}
	else
	{
		#ifdef USE_LEDS
		led_toggle(1);
		#endif

        DPRINT("TX CCA FAIL");
	}

	tx = 0;
}

int main(void) {
	timer_event event;

	// Initialize the OSS-7 Stack
	d7aoss_init(buffer, 128, buffer, 128);
	trans_set_tx_callback(&tx_callback);

	event.next_event = SEND_INTERVAL_MS;
	event.f = &start_tx;

    DPRINT("endpoint started");


	alp_template.op = ALP_OP_RESP_DATA;
	alp_template.data = (uint8_t*) &data_template;

	data_template.file_id = 32;
	data_template.start_byte_offset = 0;
	data_template.bytes_accessing = 4;
	data_template.data = data;

	timer_add_event(&event);

	// Log the device id
    #ifdef UART
	log_print_data(device_id, 8);
    #endif

	system_watchdog_init(0x0020, 0x03);
	system_watchdog_timer_start();

	while(1)
	{
		if (add_tx_event)
		{
			add_tx_event = false;
			timer_add_event(&event);
		}

		system_lowpower_mode(0,1);
	}
}
