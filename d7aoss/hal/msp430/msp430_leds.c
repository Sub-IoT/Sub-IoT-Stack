/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "msp430_addresses.h"
#include "../leds.h"
#include "platforms/platform.h"

//#include "inc/hw_memmap.h"
#include "driverlib/5xx_6xx/gpio.h"
#include "driverlib/5xx_6xx/wdt.h"

void led_init()
{
    GPIO_setAsOutputPin(OUTPUT1_BASEADDRESS, OUTPUT1_PORT, OUTPUT1_PIN);
    GPIO_setAsOutputPin(OUTPUT2_BASEADDRESS, OUTPUT2_PORT, OUTPUT2_PIN);
    GPIO_setAsOutputPin(OUTPUT3_BASEADDRESS, OUTPUT3_PORT, OUTPUT3_PIN);
}


void led_on(unsigned char led_nr)
{
    switch (led_nr)
    {
        case 1:
            GPIO_setOutputHighOnPin(OUTPUT1_BASEADDRESS, OUTPUT1_PORT, OUTPUT1_PIN);
            break;
        case 2:
            GPIO_setOutputHighOnPin(OUTPUT2_BASEADDRESS, OUTPUT2_PORT, OUTPUT2_PIN);
            break;
        case 3:
            GPIO_setOutputHighOnPin(OUTPUT3_BASEADDRESS, OUTPUT3_PORT, OUTPUT3_PIN);
            break;
    }
}

void led_off(unsigned char led_nr)
{
    switch (led_nr)
    {
		case 1:
			GPIO_setOutputLowOnPin(OUTPUT1_BASEADDRESS, OUTPUT1_PORT, OUTPUT1_PIN);
			break;
		case 2:
			GPIO_setOutputLowOnPin(OUTPUT2_BASEADDRESS, OUTPUT2_PORT, OUTPUT2_PIN);
			break;
		case 3:
			GPIO_setOutputLowOnPin(OUTPUT3_BASEADDRESS, OUTPUT3_PORT, OUTPUT3_PIN);
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
