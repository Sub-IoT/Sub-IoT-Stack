/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *
 */

#include <trans/trans.h>
#include <dll/dll.h>

#include <string.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <framework/log.h>
#include <msp430.h>

#define INTERRUPT_RTC 		(1 << 3)

uint8_t dataSend = 0;
static uint8_t interrupt_flags = 0;
bool IsReceived = false;

dll_channel_scan_t scan_cfg1 = {
		0x25,
		FrameTypeForegroundFrame,
		100,
		0
};

dll_channel_scan_series_t scan_series_cfg;

void start_rx()
{
	dll_channel_scan_series(&scan_series_cfg);
	led_on(2);
}

void stop_rx()
{
	dll_stop_channel_scan();
	led_off(2);
}


void start_tx()
{

	//log_print_string("start_tx");
	dataSend++;
    trans_tx_foreground_frame(&dataSend, sizeof(dataSend), 0xFF, 0x18, 0x00);

}

void rx_callback(dll_rx_res_t* rx_res)
{
	IsReceived = true;
	stop_rx();
}

void tx_callback(Trans_Tx_Result result)
{
	switch(result){
			case TransPacketSent:
				log_print_string("TX successful");
				led_toggle(1);
				break;
			case TransTCAFail:
				log_print_string("T_CA is less than the T_G");
				led_toggle(2);
				break;
			case TransPacketFail:
				log_print_string("Problem with the CCA");
				led_toggle(3);
				break;
		}

}

void main(void) {
	system_init();

	trans_init();
	trans_set_tx_callback(&tx_callback);
	dll_init();
	dll_set_rx_callback(&rx_callback);

	dll_channel_scan_t scan_confgs[1];
	scan_confgs[0] = scan_cfg1;

	scan_series_cfg.length = 1;
	scan_series_cfg.values = scan_confgs;

	start_rx(); // start listening - channel 25

	rtc_init_counter_mode();
	rtc_start();
	rtc_enable_interrupt();
	while(1)
	{
		if (INTERRUPT_RTC & interrupt_flags)
		 {
			if(IsReceived)// if a packet is received on channel 25, the tag will start sending on channel 18
			{
				start_tx();
			}

			 interrupt_flags &= ~INTERRUPT_RTC;
		}
	system_lowpower_mode(4,1);
	}
}


#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR (void)
{
    switch (RTCIV){
        case 0: break;  //No interrupts
        case 2: break;  //RTCRDYIFG
        case 4:         //RTCEVIFG
        	interrupt_flags |= INTERRUPT_RTC;
        	LPM4_EXIT;
            break;
        case 6: break;  //RTCAIFG
        case 8: break;  //RT0PSIFG
        case 10: break; //RT1PSIFG
        default: break;
    }
}

