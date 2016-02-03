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

/*! \file stm32f4_gpio.c

 *
 */


#include "hwgpio.h"



__LINK_C void __gpio_init()
{

}

__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed, uint8_t mode, unsigned int out)
{
   
    return SUCCESS;    
}

__LINK_C error_t hw_gpio_set(pin_id_t pin_id)
{
    return SUCCESS;
}

__LINK_C error_t hw_gpio_clr(pin_id_t pin_id)
{
    return SUCCESS;
}

__LINK_C error_t hw_gpio_toggle(pin_id_t pin_id)
{
    return SUCCESS;
}

__LINK_C bool hw_gpio_get_out(pin_id_t pin_id)
{
    return false;
}

__LINK_C bool hw_gpio_get_in(pin_id_t pin_id)
{
    return false;
}

static void gpio_int_callback(uint8_t pin)
{

}

__LINK_C error_t hw_gpio_configure_interrupt(pin_id_t pin_id, gpio_inthandler_t callback, uint8_t event_mask)
{

}
__LINK_C error_t hw_gpio_enable_interrupt(pin_id_t pin_id)
{
    
}

__LINK_C error_t hw_gpio_disable_interrupt(pin_id_t pin_id)
{
    
}
