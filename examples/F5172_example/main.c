/*
 * Created on: September 6, 2013
 * Authors:
 * 		jan.stevens@ieee.org
 *
 * 	Testing out the MSP430F5172 without the RF part.
 *
 */
#include <string.h>
#include <trans/trans.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <framework/log.h>
#include <framework/timer.h>
#include <msp430.h> 

/*
 * main.c
 */
int main(void) {
    // Initialize the OSS-7 Stack
	system_init();

	trans_init();
	log_print_string("We have started");
	log_print_data(device_id, 8);

	system_watchdog_init(WDTSSEL0, 0x03);
	system_watchdog_timer_start();

	while(1)
	{
		led_on(1);
		system_lowpower_mode(4,1);
	}
	
	return 0;
}
