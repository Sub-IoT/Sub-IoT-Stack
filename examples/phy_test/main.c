/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */


#include <string.h>

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

#define PACKET_LEN 19

static u8 packet[PACKET_LEN] = { 0x13, 0x50, 0xF1, 0x20, 0x59, 0x40, 0x46, 0x93, 0x21, 0xAB, 0x00, 0x31, 0x00, 0x24, 0x00, 0x00, 0x00, 0x01, 0x01 };

static u8 interrupt_flags = 0;
static u8 rtcEnabled = 0;

phy_rx_cfg_t rx_cfg = {
		0, // timeout
		0, // multiple TODO
		0x27, // spectrum ID TODO
		0, // coding scheme TODO
		0, // RSSI min filter TODO
		0x01  // sync word class TODO
};

phy_tx_cfg_t tx_cfg = {
	    0x27, // spectrum ID
		0, // coding scheme
		0, // sync word class
	    packet,
	    PACKET_LEN,
	    0, // tx_eirp
};

void start_rx()
{
	phy_rx_start(&rx_cfg); // TODO remove (use timeout/multiple)
	led_on(2);
}

void stop_rx()
{
	phy_rx_stop();
	led_off(2);
}

void tx_callback()
{
	log_print_string("tx_callback");
	led_off(3);
	start_rx(); // go back to rx mode
}

void rx_callback(phy_rx_res_t* res)
{
	if(memcmp(res->data, packet, res->len) != 0)
	{
		__no_operation(); // TODO assert
		log_print_string("!!! unexpected packet data");
		led_off(3);
		led_toggle(1);
	}
	else
	{
		char text[14] = "RX OK     dBm";
		u8 i = 6;
		if (res->rssi < 0)
		{
			text[i++] = '-';
			res->rssi  *= -1;
		}
		if (res->rssi  >= 100)
		{
			text[i++] = '1';
			res->rssi  -= 100;
		}

		text[i++] = 0x30 + (res->rssi / 10);
		text[i++] = 0x30 + (res->rssi % 10);
		log_print_string((char*)&text);
		led_toggle(3);
	}

	start_rx();
}

void main(void) {
	system_init();
	button_enable_interrupts();

	rtc_init_counter_mode();
	rtc_start();

	phy_init();
	phy_set_tx_callback(&tx_callback);
	phy_set_rx_callback(&rx_callback);

	start_rx();

	while(1)
	{
        if(INTERRUPT_BUTTON1 & interrupt_flags)
        {
        	interrupt_flags &= ~INTERRUPT_BUTTON1;
        	led_on(3);

        	if(phy_is_rx_in_progress() == true)
        		stop_rx();

        	phy_tx(&tx_cfg);

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
			stop_rx();
			phy_tx(&tx_cfg);
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
