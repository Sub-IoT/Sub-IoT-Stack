/*
 *  Created on: Feb 2, 2013
 *  Authors:
 * 		maarten.weyn@artesis.be
 */


#include <string.h>
#include <nwl/nwl.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <log.h>
#include <timer.h>


dll_channel_scan_t scan_cfg1 = {
		0x10,
		FrameTypeBackgroundFrame,
		1000,
		0
};

dll_channel_scan_series_t scan_series_cfg;

void rx_callback(nwl_rx_res_t* rx_res)
{
	log_print_string("RX CB");
	if (rx_res->frame_type == FrameTypeBackgroundFrame)
	{
		nwl_background_frame_t* frame = (nwl_background_frame_t*) rx_res->frame;
		if (frame->bpid == BPID_AdvP)
		{
			AdvP_Data* data = (AdvP_Data*) frame->protocol_data;

			log_print_string("AdvP_Data");
		}

	}
}

start_rx()
{
	dll_channel_scan_series(&scan_series_cfg);
}


void main(void) {
	system_init();
	button_enable_interrupts();

	nwl_init();
	//nwl_set_tx_callback(&tx_callback);
	nwl_set_rx_callback(&rx_callback);

	dll_channel_scan_t scan_confgs[1];
	scan_confgs[0] = scan_cfg1;

	scan_series_cfg.length = 0;
	scan_series_cfg.values = scan_confgs;

	log_print_string("started");


	start_rx();

	while(1)
	{
		system_lowpower_mode(4,1);
	}
}

