/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include <string.h>
#include <stdio.h>

#include <d7aoss.h>

#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <hal/system.h>
#include <framework/log.h>

#include "interrupts.h"

static uint8_t rtcEnabled = 0;

static uint8_t buffer[16] = {0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

/*
 */
phy_tx_cfg_t tx_cfg = {
	.spectrum_id=0x90,
	.sync_word_class=1,
	.eirp=0,
	.length=16,
	.data=buffer
};

phy_rx_cfg_t rx_cfg = {
	.spectrum_id=0x90,
	.sync_word_class=1,
	.length=0,
	.timeout=0
};

void start_rx(void)
{
	phy_rx(&rx_cfg);
}

void start_tx(void)
{
	phy_idle();
	phy_tx(&tx_cfg);
}

void rx_callback(phy_rx_data_t* rx_data)
{
	if(memcmp(buffer, rx_data->data, sizeof(buffer)) == 0)
		led_toggle(1);
}

int main(void)
{
	system_init();
    rtc_init_counter_mode();
    rtc_start();

	//test_fec_encoding();
	//test_fec_decoding();

	button_enable_interrupts();


	phy_init();
	phy_set_rx_callback(rx_callback);

	while(1) {
        if (INTERRUPT_BUTTON1 & interrupt_flags) {
        	interrupt_flags &= ~INTERRUPT_BUTTON1;
//        	led_toggle(1);

        	start_tx();

        	button_clear_interrupt_flag();
        	button_enable_interrupts();
        }

        if (INTERRUPT_BUTTON3 & interrupt_flags) {
			interrupt_flags &= ~INTERRUPT_BUTTON3;

			if (rtcEnabled) {
				rtcEnabled = 0;
				rtc_disable_interrupt();
			} else {
				rtcEnabled = 1;
				rtc_enable_interrupt();
			}

			button_clear_interrupt_flag();
			button_enable_interrupts();
        }

        if (INTERRUPT_RTC & interrupt_flags) {
        	interrupt_flags &= ~INTERRUPT_RTC;
        	led_toggle(0);

        	start_tx();
        }

        if (!phy_is_rx_in_progress() && !phy_is_tx_in_progress()) {
        	start_rx();
        }

        if (phy_is_rx_in_progress())
        {
        	led_on(2);
        } else {
        	led_off(2);
        }

        if (phy_is_tx_in_progress())
        {
        	led_on(3);
        } else {
        	led_off(3);
        }


//		system_lowpower_mode(4,1);
	}
	return 0;
}

