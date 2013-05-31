/*
 * trans.h
 *
 *  Created on: 5 - mai.-2013
 *      Author: Maarten Weyn
 */

#ifndef PRES_H_
#define PRES_H_


#define SETTING_SELECTOR_SLEEP_SCHED 1 << 15
#define SETTING_SELECTOR_HOLD_SCHED 1 << 14
#define SETTING_SELECTOR_BEACON_SCHED 1 << 13
#define SETTING_SELECTOR_GATEWAY 1 << 11
#define SETTING_SELECTOR_SUBCONTR 1 << 10
#define SETTING_SELECTOR_ENDPOINT 1 << 9
#define SETTING_SELECTOR_BLINKER 1 << 8
#define SETTING_SELECTOR_345_WAY_TRANSFER 1 << 7
#define SETTING_SELECTOR_2_WAY_TRANSFER 1 << 6
#define SETTING_SELECTOR_FEC_TX 1 << 5
#define SETTING_SELECTOR_FEC_RX 1 << 4
#define SETTING_SELECTOR_BLINK_CHANNELS 1 << 3
#define SETTING_SELECTOR_HI_RATE_CHANNELS 1 << 2
#define SETTING_SELECTOR_NORMAL_CHANNELS 1 << 1

#include "../types.h"
#include "../hal/system.h"

// ADD .fs			: {} > FLASH to .cmd file to use filesystem

#pragma DATA_SECTION(filesystem, ".fs")

const uint8_t filesystem[] = {

		/* network configuration: ID=0x00  len=10, alloc=10 */
		0x00, 0x00,                                         /* virtual id */
	    0xFF,                                               /* Device Subnet */
	    0xFF,                                               /* Beacon Subnet */
	    (uint8_t) (SETTING_SELECTOR_ENDPOINT | SETTING_SELECTOR_HI_RATE_CHANNELS | SETTING_SELECTOR_NORMAL_CHANNELS),                    /* Active Setting */
	    0x00,                                               /* Default Device Flags */
	    2,                                                  /* Beacon Redudancy (attempts) */
	    0x0002//,                                     /* Hold Scan Sequence Cycles */



};

#endif /* PRES_H_ */
