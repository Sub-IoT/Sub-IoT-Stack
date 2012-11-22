/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "button.h"

#include "platforms/wizzimote.h"
//#include "platforms/artesis.h"

#include "addresses.h"
//#include "inc/hw_memmap.h"
#include "driverlib/5xx_6xx/gpio.h"
#include "driverlib/5xx_6xx/wdt.h"


void Buttons_Init()
{
    GPIO_setAsInputPin(INPUT1_BASEADDRESS, INPUT1_PORT, INPUT1_PIN);
    GPIO_setAsInputPin(INPUT2_BASEADDRESS, INPUT2_PORT, INPUT2_PIN);
    GPIO_setAsInputPin(INPUT2_BASEADDRESS, INPUT3_PORT, INPUT3_PIN);
}

void Buttons_EnableInterrupts()
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

    Buttons_ClearInterruptFlag();
}

void Buttons_DisableInterrupts()
{
    GPIO_disableInterrupt(INPUT1_BASEADDRESS, INPUT1_PORT, INPUT1_PIN);
    GPIO_disableInterrupt(INPUT2_BASEADDRESS, INPUT2_PORT, INPUT2_PIN);
    GPIO_disableInterrupt(INPUT3_BASEADDRESS, INPUT3_PORT, INPUT3_PIN);
}

void Buttons_ClearInterruptFlag()
{
    GPIO_clearInterruptFlag(INPUT1_BASEADDRESS, INPUT1_PORT, INPUT1_PIN);
	GPIO_clearInterruptFlag(INPUT2_BASEADDRESS, INPUT2_PORT, INPUT2_PIN);
	GPIO_clearInterruptFlag(INPUT3_BASEADDRESS, INPUT3_PORT, INPUT3_PIN);
}

unsigned char Button_IsActive(unsigned char button_nr)
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
