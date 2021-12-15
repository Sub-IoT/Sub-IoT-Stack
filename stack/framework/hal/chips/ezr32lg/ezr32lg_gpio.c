/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
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
 */

/*! \file efm32gg_gpio.c
 *
 *  \author daniel.vandenakker@uantwerpen.be
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */

#include "ezr32lg_mcu.h"
#include "hwgpio.h"
#include <em_gpio.h>
#include <em_cmu.h>
#include <em_bitband.h>
#include <gpiointerrupt.h>
#include <stdarg.h>
#include <assert.h>
#include <ezr32lg_chip.h>
#include "hwatomic.h"
#include "errors.h"


typedef struct
{
    gpio_inthandler_t callback;
    GPIO_Port_TypeDef interrupt_port;
} gpio_interrupt_t;
//the list of configured interrupts
static gpio_interrupt_t interrupts[16];

static uint16_t gpio_pins_configured[6];

__LINK_C void __gpio_init()
{
    for(int i = 0; i < 16; i++)
    {
    	interrupts[i].callback = 0x0;
    	interrupts[i].interrupt_port = 0xFF; //signal that a port has not yet been chosen
    }
    for(int i = 0; i < 6; i++)
    	gpio_pins_configured[i] = 0;
    CMU_ClockEnable(cmuClock_GPIO, true);

    /* Initialize GPIO interrupt dispatcher */
    GPIOINT_Init();
}

__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed, uint8_t mode, unsigned int out)
{
    if(int_allowed && (interrupts[GPIO_PIN(pin_id)].interrupt_port != 0xFF))
    	return EBUSY;

    //set the pin to be configured
    gpio_pins_configured[GPIO_PORT(pin_id)] |= (1<<GPIO_PIN(pin_id));
    
    //configure the pin itself
    GPIO_PinModeSet(GPIO_PORT(pin_id), GPIO_PIN(pin_id), mode, out);
    
    //if interrupts are allowed: set the port to use
    if(int_allowed)
      interrupts[GPIO_PIN(pin_id)].interrupt_port = GPIO_PORT(pin_id);
    
    return SUCCESS;    
}

__LINK_C error_t hw_gpio_set(pin_id_t pin_id)
{
    if(!(gpio_pins_configured[GPIO_PORT(pin_id)] & (1<<GPIO_PIN(pin_id))))
	return EOFF;
    GPIO_PinOutSet(GPIO_PORT(pin_id), GPIO_PIN(pin_id));
    return SUCCESS;
}

__LINK_C error_t hw_gpio_clr(pin_id_t pin_id)
{
    if(!(gpio_pins_configured[GPIO_PORT(pin_id)] & (1<<GPIO_PIN(pin_id))))
	return EOFF;
    GPIO_PinOutClear(GPIO_PORT(pin_id), GPIO_PIN(pin_id));
    return SUCCESS;
}

__LINK_C error_t hw_gpio_toggle(pin_id_t pin_id)
{
    if(!(gpio_pins_configured[GPIO_PORT(pin_id)] & (1<<GPIO_PIN(pin_id))))
	return EOFF;
    GPIO_PinOutToggle(GPIO_PORT(pin_id), GPIO_PIN(pin_id));
    return SUCCESS;
}

__LINK_C bool hw_gpio_get_out(pin_id_t pin_id)
{
    return (!!(gpio_pins_configured[GPIO_PORT(pin_id)] & (1<<GPIO_PIN(pin_id))))
  && GPIO_PinOutGet(GPIO_PORT(pin_id), GPIO_PIN(pin_id));
}

__LINK_C bool hw_gpio_get_in(pin_id_t pin_id)
{
    return (!!(gpio_pins_configured[GPIO_PORT(pin_id)] & (1<<GPIO_PIN(pin_id))))
  && GPIO_PinInGet(GPIO_PORT(pin_id), GPIO_PIN(pin_id));
}

static void gpio_int_callback(uint8_t pin)
{
	assert(interrupts[pin].callback != 0x0);
    pin_id_t id = PIN(interrupts[pin].interrupt_port, pin);
	//report an event_mask of '0' since the only way to check which event occurred
	//is to check the state of the pin from the interrupt handler and 
    //since the execution of interrupt handlers may be 'delayed' this method is NOT reliable.
    // TODO find out if there is no way to do this reliable on efm32gg
    interrupts[pin].callback(id,0);
}

__LINK_C error_t hw_gpio_configure_interrupt(pin_id_t pin_id, gpio_inthandler_t callback, uint8_t event_mask)
{
  if (interrupts[GPIO_PIN(pin_id)].interrupt_port != 0xFF)
	{
    if (interrupts[GPIO_PIN(pin_id)].interrupt_port != GPIO_PORT(pin_id))
			return EOFF;
	} else {
    interrupts[GPIO_PIN(pin_id)].interrupt_port = GPIO_PORT(pin_id);
	}

    if(callback == 0x0 || event_mask > (GPIO_RISING_EDGE | GPIO_FALLING_EDGE))
    	return EINVAL;

    error_t err;
    start_atomic();
	//do this check atomically: interrupts[..] callback is altered by this function
	//so the check belongs in the critical section as well
    if(interrupts[GPIO_PIN(pin_id)].callback != 0x0 && interrupts[GPIO_PIN(pin_id)].callback != callback)
	    err = EBUSY;
	else
	{
      interrupts[GPIO_PIN(pin_id)].callback = callback;
      GPIOINT_CallbackRegister(GPIO_PIN(pin_id), &gpio_int_callback);
      GPIO_IntConfig(GPIO_PORT(pin_id), GPIO_PIN(pin_id),
			!!(event_mask & GPIO_RISING_EDGE),
			!!(event_mask & GPIO_FALLING_EDGE),
			false);			
	    err = SUCCESS;
	}
    end_atomic();
    return err;
}
__LINK_C error_t hw_gpio_enable_interrupt(pin_id_t pin_id)
{
    //to be absolutely safe we should put atomic blocks around this fuction but:
    //interrupts[..].interrupt_port && interrupts[..].callback will never change once they've
    //been properly set so I think we can risk it and avoid the overhead
    if(interrupts[GPIO_PIN(pin_id)].interrupt_port != GPIO_PORT(pin_id) || interrupts[GPIO_PIN(pin_id)].callback == 0x0)
    	return EOFF;

    BITBAND_Peripheral(&(GPIO->IFC), GPIO_PIN(pin_id), 1);
    BITBAND_Peripheral(&(GPIO->IEN), GPIO_PIN(pin_id), 1);
    return SUCCESS;
}

__LINK_C error_t hw_gpio_disable_interrupt(pin_id_t pin_id)
{
    //to be absolutely safe we should put atomic blocks around this fuction but:
    //interrupts[..].interrupt_port && interrupts[..].callback will never change once they've
    //been properly set so I think we can risk it and avoid the overhead
    if(interrupts[GPIO_PIN(pin_id)].interrupt_port != GPIO_PORT(pin_id) || interrupts[GPIO_PIN(pin_id)].callback == 0x0)
	return EOFF;

    BITBAND_Peripheral(&(GPIO->IEN), GPIO_PIN(pin_id), 0);
    return SUCCESS;
}
