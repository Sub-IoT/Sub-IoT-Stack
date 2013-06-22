/*
 * This program measures the RSSI values in all normal channels for noise measurements
 * The measured RSSI value and channel number is logged over UART.
 * The rssi_logger.py script parses this data.
 *  Created on: Feb 4, 2012
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */

#include <stdint.h>

#include <d7aoss.h>

#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <hal/uart.h>
#include <framework/log.h>

#include <msp430.h>

#define INTERRUPT_RTC 		(1 << 3)
#define CHANNEL_COUNT 1


static uint8_t interrupt_flags = 0;
static uint8_t logbuffer[2];
static uint8_t spectrum_ids[CHANNEL_COUNT] = { 0x10 /*, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
											0x21, 0x23, 0x25, 0x27, 0x29, 0x2B,
											0x32, 0x3c*/ }; // TODO only normal channel classes?
static uint8_t current_spectrum_index = 0;

phy_rx_cfg_t rx_cfg = {
	0x10,
	0,
	0,
	0
};

uint8_t get_next_spectrum_id()
{
	current_spectrum_index++;
	if(current_spectrum_index == CHANNEL_COUNT)
		current_spectrum_index = 0;

	return spectrum_ids[current_spectrum_index];
}

int main(void) {
	system_init();

	rtc_init_counter_mode();
	rtc_start();
	rtc_enable_interrupt();

	phy_init();

	while(1)
	{
        if (INTERRUPT_RTC & interrupt_flags)
		{
			led_toggle(3);
			logbuffer[0] = rx_cfg.spectrum_id;
			logbuffer[1] = phy_get_rssi(get_next_spectrum_id(), 0);
			uart_transmit_message((uint8_t*)logbuffer, sizeof(logbuffer));
			rx_cfg.spectrum_id = get_next_spectrum_id();

			interrupt_flags &= ~INTERRUPT_RTC;
		}

		system_lowpower_mode(4,1); // TODO skip this to keep radio powered on?
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
