/*!
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
