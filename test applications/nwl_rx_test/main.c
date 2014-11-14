/*
 *  Created on: Feb 2, 2013
 *  Authors:
 * 		maarten.weyn@uantwerpen.be
 */


#include <string.h>
#include <d7aoss.h>
#include <dll/dll.h>
#include <framework/log.h>
#include <framework/timer.h>
#include <hal/leds.h>

#define BUFFER_LENGTH 512
#define RX_INTERVAL_MS	2000

//dll_channel_scan_t scan_cfg1 = {
//		0x1C,
//		FrameTypeBackgroundFrame,
//		20,
//		500
//};

static uint8_t spectrum_id[2] = { 0x00, 0x04};

dll_channel_scan_series_t scan_series_cfg;
uint8_t foreground_channel_id;


static timer_event event;
static timer_event event_ff;

uint8_t buffer[BUFFER_LENGTH];

static volatile uint8_t receiving = 0;

void scan_foreground_frame()
{
	log_print_string("FF Scan");
	dll_foreground_scan();
}

void rx_callback(nwl_rx_res_t* rx_res)
{
	log_print_string("RX CB");
	if (rx_res->protocol_type == ProtocolTypeAdvertisementProtocol)
	{
		led_toggle(3);

		nwl_background_frame_t* frame = (nwl_background_frame_t*) rx_res->data;
		if (frame->bpid == BPID_AdvP)
		{
			receiving = 1;
			dll_stop_channel_scan();

			uint16_t eta = MERGEUINT16(frame->protocol_data[0], frame->protocol_data[1]);
			log_print_string("ETA: %d", eta);




			//dll_set_foreground_scan_detection_timeout(0);
			//scan_foreground_frame();


			//timer_event event;
			event_ff.next_event = eta < 100 ? 0:  eta - 100;
			dll_set_foreground_scan_detection_timeout(200);
			//dll_set_scan_spectrum_id(data.channel_id);
			event_ff.f = &scan_foreground_frame;

			timer_add_event(&event_ff);
		}
	} else {
		log_print_string("FF");
		led_toggle(2);
		receiving = 0;
		//dll_foreground_scan();
	}
}

void start_rx()
{
	if (receiving == 1)
		return;


	led_on(3);
	///dll_channel_scan_series(&scan_series_cfg);

	//nwl_rx_start(uint8_t subnet, uint8_t spectrum_id[2], Protocol_Type type)
	log_print_string("BF Scan");
	dll_background_scan();

	timer_add_event(&event);
	led_off(3);

}


void main(void) {
	d7aoss_init(buffer, BUFFER_LENGTH, buffer, BUFFER_LENGTH);

	nwl_set_rx_callback(&rx_callback);

//	dll_channel_scan_t scan_confgs[1];
//	scan_confgs[0] = scan_cfg1;
//
//	scan_series_cfg.length = 0;
//	scan_series_cfg.values = scan_confgs;

	dll_set_background_scan_detection_timeout(20);
	dll_set_scan_spectrum_id(spectrum_id);
	dll_set_scan_minimum_energy(-100);

	event.f = &start_rx;
	event.next_event = RX_INTERVAL_MS;

	log_print_string("started");


	timer_add_event(&event);

	while(1)
	{
		system_lowpower_mode(0,1);
	}
}
