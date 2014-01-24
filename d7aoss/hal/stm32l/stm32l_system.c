/*
 *  Created on: May 9, 2013
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */

#include <stm32l1xx_rcc.h>

#include <hal/system.h>
#include <hal/leds.h>
#include <hal/button.h>
#include <hal/uart.h>
#include <hal/rtc.h>

#include "systick.h"

#define UDID_ADDRESS 0x1FF80050



uint8_t device_id[8]; // TODO: keep this as global?
uint8_t virtual_id[2];

void PMM_SetVCore(uint8_t level) {

}

void system_init() {
//	/* Enable CRC clock */
	systick_init();
	system_get_unique_id(device_id);
    //TODO: correct way to find virtual_id -> set by app layer
    virtual_id[0] = device_id[4] ^ device_id[5];
    virtual_id[1] = device_id[6] ^ device_id[7];

	led_init();
	button_init();
	uart_init();

}

void system_watchdog_timer_stop() {

}

void system_watchdog_timer_start() {

}

void system_watchdog_timer_reset() {

}

void system_watchdog_timer_enable_interrupt() {

}

void system_watchdog_timer_init(unsigned char clockSelect,
		unsigned char clockDivider) {

}

void system_watchdog_init(unsigned char clockSelect, unsigned char clockDivider) {

}

void system_lowpower_mode(unsigned char mode, unsigned char enableInterrupts) {

}

void system_get_unique_id(unsigned char *tagId) {
	uint8_t* udid = (uint8_t*) UDID_ADDRESS;
	unsigned char i;
	for (i = 0; i < 8; i++) {
		tagId[i] = udid[i];
	}

}
