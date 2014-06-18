/*
 *  Created on: Feb 2, 2013
 *  Authors:
 * 		maarten.weyn@uantwerpen.be
 */


#include <string.h>
#include <nwl/nwl.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <framework/log.h>
#include <framework/timer.h>

dll_channel_scan_t scan_cfg1 = {
		0x1C,
		FrameTypeBackgroundFrame,
		20,
		500
};

dll_channel_scan_series_t scan_series_cfg;
uint8_t foreground_channel_id;


static timer_event event;

static volatile uint8_t receiving = 0;

static char *i2a(unsigned i, char *a, unsigned r)
{
	if (i/r > 0) a = i2a(i/r,a,r);
	*a = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i%r];
	return a+1;
}

char *itoa(int i, char *a, int r)
{
	if ((r < 2) || (r > 36)) r = 10;
	if (i < 0)
	{
		*a = '-';
		*i2a(-(unsigned)i,a+1,r) = 0;
	}
	else *i2a(i,a,r) = 0;
	return a;
}

void scan_foreground_frame()
{
	log_print_string("FF Scan");
	dll_foreground_scan();
}

void rx_callback(nwl_rx_res_t* rx_res)
{
	log_print_string("RX CB");
	if (rx_res->protocol_type == ProtocolTypeBackgroundProtocol)
	{
		led_toggle(3);

		nwl_background_frame_t* frame = (nwl_background_frame_t*) rx_res->data;
		if (frame->bpid == BPID_AdvP)
		{
			dll_stop_channel_scan();
			AdvP_Data data;

			//data.channel_id = frame->protocol_data[0];
			data.eta = (frame->protocol_data[0] << 8) | (frame->protocol_data[1] & 0xFF);

			log_print_string("AdvP_Data");
			char msg[8];
			itoa(data.eta, msg, 10);
			log_print_string(msg);


			dll_set_foreground_scan_detection_timeout(data.eta * 2);
			scan_foreground_frame();
			//timer_event event;
			//event.next_event = data.eta < 100 ? 0:  data.eta - 100;
			//foreground_channel_id = data.channel_id;
			//dll_set_foreground_scan_detection_timeout(200);
			//dll_set_scan_spectrum_id(data.channel_id);
			//event.f = &scan_foreground_frame;

			//timer_add_event(&event);
		}
	} else {
		log_print_string("FF");
		led_toggle(2);
		dll_foreground_scan();
	}
}

void start_rx()
{
	if (receiving == 1)
		return;


	led_on(3);
	///dll_channel_scan_series(&scan_series_cfg);

	dll_background_scan();

	timer_add_event(&event);
	led_off(3);

}


void main(void) {
	system_init();
	//button_enable_interrupts();

	nwl_init();
	//nwl_set_tx_callback(&tx_callback);
	nwl_set_rx_callback(&rx_callback);

	dll_channel_scan_t scan_confgs[1];
	scan_confgs[0] = scan_cfg1;

	scan_series_cfg.length = 0;
	scan_series_cfg.values = scan_confgs;

	dll_set_background_scan_detection_timeout(20);
	dll_set_scan_spectrum_id(0x1C);
	dll_set_scan_minimum_energy(-100);

	event.f = &start_rx;
	event.next_event = 500;

	log_print_string("started");


	timer_add_event(&event);

	while(1)
	{
		system_lowpower_mode(3,1);
	}
}
