/*
 *  Created on: May 9, 2013
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */
#define assert_param(expr) ((void)0)
#include <stm32l1xx_rcc.h>
#include <stm32l1xx_pwr.h>
#include <misc.h>

#include <system.h>
#include <leds.h>
#include <button.h>
#include <uart.h>
#include <rtc.h>
#include <timer.h>

#include "systick.h"

#define UDID_ADDRESS 0x1FF80050

uint8_t device_id[8]; // TODO: keep this as global?
uint8_t virtual_id[2];

void PMM_SetVCore(uint8_t level) {

}

void system_init() {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
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
	uint8_t pwrMode = enableInterrupts ? PWR_STOPEntry_WFI : PWR_STOPEntry_WFE;
	switch (mode) {
	case 4:
		//STOP mode
        PWR_EnterSTOPMode(PWR_Regulator_ON, pwrMode);
		break;
	case 3:
		// Low Power sleep?
		break;
	case 2:
		// Low power run?
		break;
	case 1:
		// 4MHz run?
		break;
	case 0:
		// full speed run?
	default:
		break;
	}

}

void system_get_unique_id(unsigned char *tagId) {
	uint8_t* udid = (uint8_t*) UDID_ADDRESS;
	unsigned char i;
	for (i = 0; i < 8; i++) {
		tagId[i] = udid[i];
	}

}
