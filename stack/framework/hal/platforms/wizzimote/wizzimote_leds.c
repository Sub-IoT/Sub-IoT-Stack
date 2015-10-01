/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file cc430_leds.c
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author daniel.vandenakker@uantwerpen.be
 *
 */

#include "hwgpio.h"
#include "hwleds.h"
#include "platform.h"
#include <debug.h>

#if HW_NUM_LEDS != 3
    #error HW_NUM_LEDS does not match the expected value. Update platform.h or platform_leds.c
#endif
static pin_id_t leds[ HW_NUM_LEDS ];

void __led_init()
{
    leds[0] = P1_7;
    leds[1] = P3_7;
    leds[2] = P3_6;
    for(int i = 0; i < HW_NUM_LEDS; i++)
    {
        error_t err = hw_gpio_configure_pin(leds[i], GPIO_MODE_OUTPUT);
        assert(err == SUCCESS);
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
