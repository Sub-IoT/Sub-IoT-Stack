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
#include <framework/log.h>
#include <framework/timer.h>
#include <hal/crc.h>
#include <msp430.h> // TODO


#define INTERRUPT_BUTTON1 	(1)
#define INTERRUPT_BUTTON2 	(1 << 1)
#define INTERRUPT_BUTTON3 	(1 << 2)
#define INTERRUPT_RTC 		(1 << 3)

static uint16_t counter = 0;

static uint8_t interrupt_flags = 0;
static uint8_t rtcEnabled = 0;


static uint8_t tx = 0;

dll_channel_scan_t scan_cfg1 = {
		0x1C,
		FrameTypeForegroundFrame,
		1000,
		0
};

//dll_channel_scan_t scan_cfg1 = {
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
	if (tx)
		return;

	tx = 1;
	stop_rx();
	led_on(3);

	dll_ff_tx_cfg_t cfg;
	cfg.eirp = 0;
	cfg.spectrum_id = 0x1C;
	cfg.subnet = 0xFF;

	dll_create_foreground_frame((uint8_t*)&counter, sizeof(counter), &cfg);
	//dll_csma(1);
	dll_tx_frame();
}

void tx_callback(Dll_Tx_Result result)
{
	if(result == DLLTxResultOK)
	{
		counter++;
		led_off(3);
		log_print_string("TX OK");
	}
	else if (result == DLLTxResultCCA1Fail)
	{
		led_toggle(1);
		log_print_string("TX CCA 1 FAIL");
	}
	else if (result == DLLTxResultCCA2Fail)
		{
			led_toggle(1);
			log_print_string("TX CCA 2 FAIL");
		} else
	{
		led_toggle(1);
		log_print_string("TX FAIL");
	}

	start_rx();
	tx = 0;
}

void rx_callback(dll_rx_res_t* rx_res)
{
	led_toggle(3);
	log_print_string("RX CB");
}

void main(void) {
	system_init();
	button_enable_interrupts();

	rtc_init_counter_mode();
	rtc_start();

	/*
	CRC test

	//u8 msg[] = {0x31, 0xFF, 0x41, 0x43, 0x54, 0x49, 0x56, 0x45, 0x20, 0x42, 0x54, 0x4E, 0x31, 0x00, 0x00};
	u8 msg[] = {0x29,	0x64,	0xFF,	0x61,	0x15,	0x34,	0xD0,	0x9F,	0x9E,	0x88,	0xEA,	0x2D,
			0x2A,	0x87,	0x5C,	0x39,	0x2D,	0xF5,	0xB4,	0x5C,	0x3F,	0xF1,	0x03,	0x81,
			0xD4,	0x13,	0xFA,	0x32,	0x90,	0x75,	0x67,	0x3E,	0x48,	0xDA,	0x84,	0x90,
			0xF4,	0x2C,	0xC8,	0xD1,	0x54};

	u8 size = 41;
	u16 crc = crc_calculate(msg, size);
	u16 crc2 = crc16(msg, size);
	u16 crc3 = CRCCCITT(msg, size, 0xffff, 0);

	u8 text[] = "crc = 0x0000\0";
	u8 temp = crc & 0x0F;
	text[11] = (temp <= 9) ? 0x30 + temp : (0x41-10) + temp;
	temp = (crc >> 4) & 0x0F;
	text[10] = (temp <= 9) ? 0x30 + temp : (0x41-10) + temp;
	temp = (crc >> 8) & 0x0F;
	text[9] = (temp <= 9) ? 0x30 + temp : (0x41-10) + temp;
	temp = (crc >> 12) & 0x0F;
	text[8] = (temp <= 9) ? 0x30 + temp : (0x41-10) + temp;
	log_print_string(text);
	*/

	dll_init();
	dll_set_tx_callback(&tx_callback);
	dll_set_rx_callback(&rx_callback);

	dll_channel_scan_t scan_confgs[1];
	scan_confgs[0] = scan_cfg1;

	scan_series_cfg.length = 0;
	scan_series_cfg.values = scan_confgs;

	log_print_string("started");

	start_rx();

	while(1)
	{

        if(INTERRUPT_BUTTON1 & interrupt_flags)
        {
        	interrupt_flags &= ~INTERRUPT_BUTTON1;

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
        	log_print_string("rtc");
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
		log_print_string("button_disable_interrupts");
		button_disable_interrupts();
		// TODO: debouncing
		LPM4_EXIT;
	}
}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR (void)
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
