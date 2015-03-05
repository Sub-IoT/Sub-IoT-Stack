/*! \file cc430_leds.c
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
 *  \author daniel.vandenakker@uantwerpen.be
 *
 */

#include "hwgpio.h"
#include "hwleds.h"
#include "platform.h"
#include <assert.h>

#if HW_NUM_LEDS != 3
    #error HW_NUM_LEDS does not match the expected value. Update platform.h or platform_leds.c
#endif
static pin_id_t leds[ HW_NUM_LEDS ];

void __led_init()
{
    leds[0] = P1_7;
    leds[1] = P3_7;
    leds[1] = P3_6;
    for(int i = 0; i < HW_NUM_LEDS; i++)
    {
        //error_t err = hw_gpio_configure_pin(leds[i], false, gpioModePushPull, 0);
        //assert(err == SUCCESS);
    }
}

void led_on(uint8_t led_nr)
{
    if(led_nr < HW_NUM_LEDS)
        hw_gpio_set(leds[led_nr]);
}

void led_off(unsigned char led_nr)
{
    if(led_nr < HW_NUM_LEDS)
        hw_gpio_clr(leds[led_nr]);
}

void led_toggle(unsigned char led_nr)
{
    if(led_nr < HW_NUM_LEDS)
        hw_gpio_toggle(leds[led_nr]);
}
