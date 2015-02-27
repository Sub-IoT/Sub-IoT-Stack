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
 * 	Example Phy logger which can be used to sniff your network
 *
 * 	add the link to d7aoss library in de lnk_*.cmd file, e.g. -l "../../../d7aoss/Debug/d7aoss.lib"
 * 	Make sure to select the correct platform in d7aoss.h
 * 	If your platform is not present, you can add a header file in platforms and commit it to the repository.
 * 	Exclude the stub directories in d7aoss from the build when building for a device.
 *
 * 	Create the apporpriate file system settings for the FLASH system:
 *
 * 	Add following sections to the SECTIONS in .cmd linker file to use the filesystem
 *		.fs_fileinfo_bitmap : 	{} > FLASH_FS1
 *  	.fs_fileinfo: 			{} > FLASH_FS1
 *		.fs_files	: 			{} > FLASH_FS2
 *
 *	Add FLASH_FS_FI and FLASH_FS_FILES to the MEMORY section
 *  eg.
 *		FLASH                   : origin = 0x8600, length = 0x7980
 *   	FLASH_FS1               : origin = 0x8000, length = 0x0200
 *   	FLASH_FS2               : origin = 0x8200, length = 0x0400
 */


#include <d7aoss.h>
#include <framework/log.h>
#include <hal/leds.h>


static dll_channel_scan_t scan_cfg;

static uint8_t buffer[128];


void start_rx()
{
	phy_rx_cfg_t rx_cfg;
	rx_cfg.length = 0;
	rx_cfg.timeout = scan_cfg.timeout_scan_detect; // timeout
	rx_cfg.spectrum_id[0] = scan_cfg.spectrum_id[0];
	rx_cfg.spectrum_id[1] = scan_cfg.spectrum_id[1];
	rx_cfg.scan_minimum_energy = -140;
	if (scan_cfg.scan_type == FrameTypeBackgroundFrame)
	{
		rx_cfg.sync_word_class = 0;
	} else {
		rx_cfg.sync_word_class = 1;
	}

	//phy_keep_radio_on(true);

	bool phy_rx_result = phy_rx(&rx_cfg);
	if (!phy_rx_result)
	{
		log_print_string("Starting channel scan FAILED");

		led_off(2);
		led_off(3);
	}
	else
	{
		led_on(2);
	}
}

void rx_callback(phy_rx_data_t* rx_res)
{

	if (rx_res != NULL)
	{
		led_on(3);
		log_phy_rx_res(rx_res);
		//uint16_t eta = MERGEUINT16(rx_res->data[4], rx_res->data[5]);
		//log_print_string("eta: %d", eta);
		led_off(3);
	}

	start_rx();


}


int main(void) {
	// Initialize the OSS-7 Stack
	d7aoss_init(buffer, 128, buffer, 128);

	scan_cfg.spectrum_id[1] = 0x00;
	scan_cfg.spectrum_id[0] = 0x04;
	scan_cfg.scan_type = FrameTypeForegroundFrame;
	//scan_cfg.scan_type = FrameTypeBackgroundFrame;
	scan_cfg.time_next_scan = 0;
	scan_cfg.timeout_scan_detect = 0;


	phy_set_rx_callback(&rx_callback);
	
	log_print_string("started");
	start_rx();

	while(1)
	{
		system_lowpower_mode(0,1);
	}

}
