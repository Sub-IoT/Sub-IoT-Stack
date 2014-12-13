/*! \file efm32gg_leds.c
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
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */

#include "leds.h"

#include "em_cmu.h"
#include "em_gpio.h"

typedef struct
{
  GPIO_Port_TypeDef port;
  unsigned int pin;
} leds_t;

static const leds_t leds[ 2 ] = { { gpioPortE, 2 }, { gpioPortE, 3 } };

void led_init()
{
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
    for (int i = 0; i < 2; i++)
    {
      GPIO_PinModeSet(leds[i].port, leds[i].pin, gpioModePushPull, 0);
    }
}

void led_on(unsigned char led_nr)
{
    GPIO_PinOutSet(leds[led_nr].port, leds[led_nr].pin);
}

void led_off(unsigned char led_nr)
{
    GPIO_PinOutClear(leds[led_nr].port, leds[led_nr].pin);
}

void led_toggle(unsigned char led_nr)
{
    GPIO_PinOutToggle(leds[led_nr].port, leds[led_nr].pin);
}

void led_blink(unsigned char led_id)
{
  // TODO
}