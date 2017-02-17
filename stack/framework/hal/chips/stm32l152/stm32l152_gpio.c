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

/*! \file stm32l152_gpio.c

 *
 */


#include "hwgpio.h"
#include "stm32l1xx_hal_gpio.h"

typedef struct
{
    gpio_inthandler_t callback;
    uint32_t interrupt_port;
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


    /* GPIO Ports Clock Enable */
    //todo: only used clk
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Initialize GPIO interrupt dispatcher */
    //GPIOINT_Init();
}

__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed, uint8_t mode, unsigned int out)
{
    //do early-stop error checking
    if((gpio_pins_configured[pin_id.port] & (1<<pin_id.pin)))
    return EALREADY;
    else if(int_allowed && (interrupts[pin_id.pin].interrupt_port != 0xFF))
    return EBUSY;

    //set the pin to be configured
    gpio_pins_configured[pin_id.port] |= (1<<pin_id.pin);

    //configure the pin itself
    GPIO_PinModeSet(pin_id.port, pin_id.pin, mode, out);

    //if interrupts are allowed: set the port to use
    if(int_allowed)
    interrupts[pin_id.pin].interrupt_port = pin_id.port;

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
