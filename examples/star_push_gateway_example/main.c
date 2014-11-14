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
 * 	This is the Gateway example
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


#include <string.h>

#include <trans/trans.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <hal/uart.h>
#include <framework/log.h>
#include <framework/timer.h>

#define TX_EIRP 10
#define USE_LEDS

// event to create a led blink
static timer_event dim_led_event;
static bool start_channel_scan = false;
uint8_t buffer[128];

static uint8_t receive_channel[2] = {0x04, 0x00};


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
	trans_rx_query_start(0xFF, receive_channel);
}

void rx_callback(Trans_Rx_Alp_Result* rx_res)
{
	system_watchdog_timer_reset();

	blink_led();

	if (rx_res->nwl_rx_res->protocol_type == ProtocolTypeNetworkProtocol)
	{
		nwl_ff_D7ANP_t* np = (nwl_ff_D7ANP_t*) (rx_res->nwl_rx_res->data);
		uint8_t* address_ptr = NULL;
		uint8_t address_length = 0;

		switch (np->control & NWL_CONTRL_SRC_FULL)
		{
		case NWL_CONTRL_SRC_UID:
			address_ptr = &(np->d7anp_source_access_templ[0]);
			address_length = 2;
			break;
		case NWL_CONTRL_SRC_VID:
			address_ptr = &(np->d7anp_source_access_templ[0]);
			address_length = 8;
			break;
		case NWL_CONTRL_SRC_FULL:
			address_ptr = &(np->d7anp_source_access_templ[1]);

			if (np->d7anp_source_access_templ[0] & NWL_ACCESS_TEMPL_CTRL_VID)
			{
				address_length = 2;
			} else {
				address_length = 8;
			}
		}

		if (address_length == 2)
		{
			log_print_string("Received Query from :%x%x%x%x;",
								address_ptr[1] >> 4, address_ptr[1] & 0x0F);
		} else if (address_length == 8) {
			log_print_string("Received Query from :%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x;",
					address_ptr[0] >> 4, address_ptr[0] & 0x0F,
					address_ptr[1] >> 4, address_ptr[1] & 0x0F,
					address_ptr[2] >> 4, address_ptr[2] & 0x0F,
					address_ptr[3] >> 4, address_ptr[3] & 0x0F,
					address_ptr[4] >> 4, address_ptr[4] & 0x0F,
					address_ptr[5] >> 4, address_ptr[5] & 0x0F,
					address_ptr[6] >> 4, address_ptr[6] & 0x0F,
					address_ptr[7] >> 4, address_ptr[7] & 0x0F);
		}
	log_print_string("RSS: %d dBm", rx_res->nwl_rx_res->dll_rx_res->rssi);
	//log_print_string("Netto Link: %d dBm", rx_res->nwl_rx_res->dll_rx_res->rssi  - ((frame->control & 0x3F) - 32));

	log_print_string("D7AQP received - ALP data:");
	//log_print_data(rx_res->d7aqp_command.alp_data, rx_res->d7aqp_command.alp_length);
	}

	start_channel_scan = true;

}


int main(void) {
	// Initialize the OSS-7 Stack
	system_init(buffer, 128, buffer, 128);

	// Currently we address the Transport Layer for RX, this should go to an upper layer once it is working.
	trans_init();
	trans_set_alp_rx_callback(&rx_callback);
	// The initial Tca for the CSMA-CA in
	dll_set_initial_t_ca(200);

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
