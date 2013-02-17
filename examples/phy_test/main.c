/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include <string.h>
#include <stdio.h>

#include <phy/phy.h>

#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <log.h>

#define INTERRUPT_BUTTON1 	(1)
#define INTERRUPT_BUTTON2 	(1 << 1)
#define INTERRUPT_BUTTON3 	(1 << 2)
#define INTERRUPT_RTC 		(1 << 3)

static u8 interrupt_flags = 0;
static u8 rtcEnabled = 0;

static uint8_t buffer[48] = {0x30, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
		0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

phy_tx_cfg tx_cfg = {
	0x10,
	1,
	0,
	48,
	buffer
};

phy_rx_cfg rx_cfg = {
	0x10,
	1,
	0,
	0
};

phy_rx_data rx_data;

void main(void)
{
	system_init();
	button_enable_interrupts();

    rtc_init_counter_mode();
    rtc_start();

	phy_init();

	while(1) {
        if (INTERRUPT_BUTTON1 & interrupt_flags) {
        	interrupt_flags &= ~INTERRUPT_BUTTON1;

        	phy_rx_stop();
        	phy_tx(&tx_cfg);

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

        	phy_rx_stop();
        	phy_tx(&tx_cfg);
        }

        if (phy_read(&rx_data)) {
        	if(memcmp(buffer, rx_data.data, sizeof(buffer)) == 0)
        		led_toggle(1);
        }

        if (!phy_is_rx_in_progress() && !phy_is_tx_in_progress()) {
        	phy_rx_start(&rx_cfg);
        }

        if (phy_is_rx_in_progress())
        {
        	led_on(2);
        } else {
        	led_off(2);
        }

        if (phy_is_tx_in_progress()) {
        	led_on(3);
        } else {
        	led_off(3);
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
