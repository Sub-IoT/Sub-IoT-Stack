/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include <stdbool.h>

#include "../button.h"

#include "driverlib/5xx_6xx/gpio.h"
#include "driverlib/5xx_6xx/wdt.h"

#include "platforms/platform.h"
#include "msp430_addresses.h"


void button_init()
{

    GPIO_setAsInputPin(INPUT1_BASEADDRESS, INPUT1_PORT, INPUT1_PIN);
    GPIO_setAsInputPin(INPUT2_BASEADDRESS, INPUT2_PORT, INPUT2_PIN);
    GPIO_setAsInputPin(INPUT2_BASEADDRESS, INPUT3_PORT, INPUT3_PIN);

}

void button_enable_interrupts()
{

    GPIO_enableInterrupt(INPUT1_BASEADDRESS, INPUT1_PORT, INPUT1_PIN);
    GPIO_enableInterrupt(INPUT2_BASEADDRESS, INPUT2_PORT, INPUT2_PIN);
    GPIO_enableInterrupt(INPUT3_BASEADDRESS, INPUT3_PORT, INPUT3_PIN);

    GPIO_interruptEdgeSelect(INPUT1_BASEADDRESS, INPUT1_PORT, INPUT1_PIN,
        GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(INPUT2_BASEADDRESS, INPUT2_PORT, INPUT2_PIN,
            GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(INPUT3_BASEADDRESS, INPUT3_PORT, INPUT3_PIN,
            GPIO_HIGH_TO_LOW_TRANSITION);

    button_clear_interrupt_flag();

}

void button_disable_interrupts()
{

    GPIO_disableInterrupt(INPUT1_BASEADDRESS, INPUT1_PORT, INPUT1_PIN);
    GPIO_disableInterrupt(INPUT2_BASEADDRESS, INPUT2_PORT, INPUT2_PIN);
    GPIO_disableInterrupt(INPUT3_BASEADDRESS, INPUT3_PORT, INPUT3_PIN);

}

void button_clear_interrupt_flag()
{

    GPIO_clearInterruptFlag(INPUT1_BASEADDRESS, INPUT1_PORT, INPUT1_PIN);
	GPIO_clearInterruptFlag(INPUT2_BASEADDRESS, INPUT2_PORT, INPUT2_PIN);
	GPIO_clearInterruptFlag(INPUT3_BASEADDRESS, INPUT3_PORT, INPUT3_PIN);

}

unsigned char button_is_active(unsigned char button_nr)
{
    switch (button_nr)
    {
        case 1:
            return (GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(INPUT1_BASEADDRESS, INPUT1_PORT, INPUT1_PIN));
        case 2:
            return (GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(INPUT2_BASEADDRESS, INPUT2_PORT, INPUT2_PIN));
        case 3:
            return (GPIO_INPUT_PIN_LOW == GPIO_getInputPinValue(INPUT3_BASEADDRESS, INPUT3_PORT, INPUT3_PIN));
        default:
        	return false;
    }

}
