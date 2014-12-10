/*!
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *     	glenn.ergeerts@uantwerpen.be
 *
 */

#include "cc430_addresses.h"
#include "../leds.h"
#include "platforms/platform.h"
#include "../../framework/timer.h"

//#include "inc/hw_memmap.h"
#include "driverlib/5xx_6xx/gpio.h"
#include "driverlib/5xx_6xx/wdt.h"

static void led_dim();

static timer_event dim_led_event = { .next_event = 50, .f = &led_dim };
static uint8_t blink_led_mask = 0;

void led_init()
{
    GPIO_setAsOutputPin(OUTPUT1_BASEADDRESS, OUTPUT1_PORT, OUTPUT1_PIN);
    GPIO_setAsOutputPin(OUTPUT2_BASEADDRESS, OUTPUT2_PORT, OUTPUT2_PIN);
    GPIO_setAsOutputPin(OUTPUT3_BASEADDRESS, OUTPUT3_PORT, OUTPUT3_PIN);

    led_off(1);
    led_off(2);
    led_off(3);
}


void led_on(unsigned char led_nr)
{
    switch (led_nr)
    {
        case 1:
        	OUTPUT1_TYPE == 0 ? GPIO_setOutputHighOnPin(OUTPUT1_BASEADDRESS, OUTPUT1_PORT, OUTPUT1_PIN): GPIO_setOutputLowOnPin(OUTPUT1_BASEADDRESS, OUTPUT1_PORT, OUTPUT1_PIN);
            break;
        case 2:
        	OUTPUT2_TYPE == 0 ? GPIO_setOutputHighOnPin(OUTPUT2_BASEADDRESS, OUTPUT2_PORT, OUTPUT2_PIN): GPIO_setOutputLowOnPin(OUTPUT2_BASEADDRESS, OUTPUT2_PORT, OUTPUT2_PIN);
        	 break;
        case 3:
        	OUTPUT3_TYPE == 0 ? GPIO_setOutputHighOnPin(OUTPUT3_BASEADDRESS, OUTPUT3_PORT, OUTPUT3_PIN): GPIO_setOutputLowOnPin(OUTPUT3_BASEADDRESS, OUTPUT3_PORT, OUTPUT3_PIN);
        	break;
    }
}

void led_off(unsigned char led_nr)
{
    switch (led_nr)
    {
		case 1:
			OUTPUT1_TYPE == 0 ? GPIO_setOutputLowOnPin(OUTPUT1_BASEADDRESS, OUTPUT1_PORT, OUTPUT1_PIN): GPIO_setOutputHighOnPin(OUTPUT1_BASEADDRESS, OUTPUT1_PORT, OUTPUT1_PIN);
			break;
		case 2:
			OUTPUT2_TYPE == 0 ? GPIO_setOutputLowOnPin(OUTPUT2_BASEADDRESS, OUTPUT2_PORT, OUTPUT2_PIN): GPIO_setOutputHighOnPin(OUTPUT2_BASEADDRESS, OUTPUT2_PORT, OUTPUT2_PIN);
			 break;
		case 3:
			OUTPUT3_TYPE == 0 ? GPIO_setOutputLowOnPin(OUTPUT3_BASEADDRESS, OUTPUT3_PORT, OUTPUT3_PIN): GPIO_setOutputHighOnPin(OUTPUT3_BASEADDRESS, OUTPUT3_PORT, OUTPUT3_PIN);
			break;
    }
}

void led_toggle(unsigned char led_nr)
{
    switch (led_nr)
    {
		case 1:
			GPIO_toggleOutputOnPin(OUTPUT1_BASEADDRESS, OUTPUT1_PORT, OUTPUT1_PIN);
			break;
		case 2:
			GPIO_toggleOutputOnPin(OUTPUT2_BASEADDRESS, OUTPUT2_PORT, OUTPUT2_PIN);
			break;
		case 3:
			GPIO_toggleOutputOnPin(OUTPUT3_BASEADDRESS, OUTPUT3_PORT, OUTPUT3_PIN);
			break;
    }
}


void led_blink(unsigned char led_id)
{
	led_on(led_id);

	blink_led_mask = blink_led_mask | (1 << led_id);

	timer_add_event(&dim_led_event);
}

static void led_dim()
{
	if ((blink_led_mask & (uint8_t) BIT1) == (uint8_t) BIT1)
	{
		led_off(1);
		blink_led_mask = blink_led_mask & !BIT1;
	}
	if ((blink_led_mask & (uint8_t) BIT2) == (uint8_t) BIT2)
	{
		led_off(2);
		blink_led_mask = blink_led_mask & !BIT2;
	}
	if ((blink_led_mask & (uint8_t) BIT3) == (uint8_t) BIT3)
	{
		led_off(3);
		blink_led_mask = blink_led_mask & !BIT3;
	}
}
