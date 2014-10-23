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
#include <msp430.h>
#include <hal/leds.h>

#define SEND_INTERVAL_MS 2000
#define TX_EIRP 10

// Macro which can be removed in production environment
#define USE_LEDS

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

//1DFF3E01469321AB0033001EC000400D012020000200040316AABB7AF2
//1DFF2A01469321AB0033001EC000400D00202000020004BBAA16033DB1
static uint8_t frame_data[] = {0x1D,0xFF,0x3E,0x01,0x46,0x93,0x21,0xAB,0x00,0x33,0x00,0x1E,0xC0,0x00,0x40,0x0D,0x01,0x20,0x20,0x00,0x02,0x00,0x04,0x03,0x16,0xAA,0xBB,0x7A,0xF2};
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

		//data[0] = counter >> 8;
		//data[1] = counter & 0xFF;
		data[0] = 0x03;
		data[1] = 0x16;
		data[2] = 0xAA;
		data[3] = 0xBB;

		//alp_create_structure_for_tx(ALP_REC_FLG_TYPE_UNSOLICITED, 0, 1, &alp_template);
		//trans_tx_query(NULL, 0xFF, send_channel, TX_EIRP);

		queue_clear(&tx_queue);
		queue_push_u8_array(&tx_queue, frame_data, 0x1D);

		phy_tx_cfg_t phy_cfg;
		phy_cfg.eirp = 10;
		memcpy(phy_cfg.spectrum_id, send_channel, 2);
		phy_cfg.sync_word_class = 1;

		if (phy_tx(&phy_cfg))
		{
			log_print_string("send ok");
		} else {

			log_print_string("send fail");
		}
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
	d7aoss_init(buffer, 128, buffer, 128);
	trans_set_tx_callback(&tx_callback);

	event.next_event = SEND_INTERVAL_MS;
	event.f = &start_tx;

	log_print_string("endpoint started");

	alp_template.op = ALP_OP_RESP_DATA;
	alp_template.data = (uint8_t*) &data_template;

	data_template.file_id = 32;
	data_template.start_byte_offset = 2;
	data_template.bytes_accessing = 4;
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

		system_lowpower_mode(0,1);
	}
}
