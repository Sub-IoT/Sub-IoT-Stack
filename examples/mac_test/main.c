/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */


#include <string.h>

#include <dll/dll.h>

#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <log.h>


#define INTERRUPT_BUTTON1 	(1)
#define INTERRUPT_BUTTON2 	(1 << 1)
#define INTERRUPT_BUTTON3 	(1 << 2)
#define INTERRUPT_RTC 		(1 << 3)

static u16 counter = 0;

static u8 interrupt_flags = 0;
static u8 rtcEnabled = 0;

dll_channel_scan_t scan_cfg1 = {
		0x10,
		FrameTypeForegroundFrame,
		1000,
		0
};
//dll_channel_scan_t scan_cfg2 = {
//		0x12,
//		FrameTypeForegroundFrame,
//		1000,
//		0
//};

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
	stop_rx();
	dll_tx_foreground_frame((u8*)&counter, sizeof(counter), 0x10);
}

void tx_callback(Dll_Tx_Result result)
{
	if(result == DLLTxResultOK)
		counter++;

	led_off(3);
	start_rx();
}

void rx_callback(dll_rx_res_t* rx_res)
{
	led_toggle(3);
	log_print_string("RX CB");

	start_rx();
}

void main(void) {
	system_init();
	button_enable_interrupts();

	rtc_init_counter_mode();
	rtc_start();

	dll_init();
	dll_set_tx_callback(&tx_callback);
	dll_set_rx_callback(&rx_callback);

	dll_channel_scan_t scan_confgs[1];
	scan_confgs[0] = scan_cfg1;
	//scan_confgs[1] = scan_cfg2;

	scan_series_cfg.length = 1;
	scan_series_cfg.values = scan_confgs;


	start_rx();

	while(1)
	{

        if(INTERRUPT_BUTTON1 & interrupt_flags)
        {
        	interrupt_flags &= ~INTERRUPT_BUTTON1;
        	led_on(3);

        	start_tx();

        	button_clear_interrupt_flag();
        	button_enable_interrupts();
        }


        if (INTERRUPT_BUTTON3 & interrupt_flags)
        {
        	interrupt_flags &= ~INTERRUPT_BUTTON3;

        	if (rtcEnabled)
			{
				rtcEnabled = 0;
				rtc_disable_interrupt();
				start_rx();
			} else {
				rtcEnabled = 1;
				rtc_enable_interrupt();
				stop_rx();
			}

        	button_clear_interrupt_flag();
        	button_enable_interrupts();
        }

        if (INTERRUPT_RTC & interrupt_flags)
		{
        	led_on(3);
        	start_tx();

			interrupt_flags &= ~INTERRUPT_RTC;
		}


		system_lowpower_mode(4,1);
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR (void)
{
	if(button_is_active(1))
		interrupt_flags |= INTERRUPT_BUTTON1;
	else
		interrupt_flags &= ~INTERRUPT_BUTTON1;

	if(button_is_active(3))
		interrupt_flags |= INTERRUPT_BUTTON3;
	else
		interrupt_flags &= ~INTERRUPT_BUTTON3;

	if(interrupt_flags != 0)
	{
		button_disable_interrupts();
		LPM4_EXIT;
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
