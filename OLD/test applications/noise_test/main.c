/*
 * This program measures the RSSI values in all normal channels for noise measurements
 * The measured RSSI value and channel number is logged over UART.
 * The rssi_logger.py script parses this data.
 *  Created on: Feb 4, 2012
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */

#include <stdint.h>

//#include <d7aoss.h>

#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <hal/uart.h>
#include <framework/log.h>
#include <phy.h>

#define CHANNEL_COUNT 1

#ifdef UART
#define DPRINT(str) log_print(str)
#define DPRINTF(...) log_print(..)
#else
#define DPRINT(str)
#define DPRINTF(...)
#endif


static uint8_t buffer[128];
static uint8_t logbuffer[2];
//static uint8_t spectrum_ids[CHANNEL_COUNT] = { 0x10 /*, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
//											0x21, 0x23, 0x25, 0x27, 0x29, 0x2B,
//											0x32, 0x3c*/ }; // TODO only normal channel classes?
//static uint8_t current_spectrum_index = 0;


//uint8_t get_next_spectrum_id()
//{
//	current_spectrum_index++;
//	if(current_spectrum_index == CHANNEL_COUNT)
//		current_spectrum_index = 0;

//	return spectrum_ids[current_spectrum_index];
//}

int main(void)
{
    system_init(buffer, sizeof(buffer), buffer, sizeof(buffer));

	phy_init();

	while(1)
	{
        led_toggle(0);
        int16_t rssi = phy_get_rssi(0, 0);
#ifndef NO_PRINTF
        DPRINTF("RSSI: %d dBm", rssi);
#endif
	}
}
