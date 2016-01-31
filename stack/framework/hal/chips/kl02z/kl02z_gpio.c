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

/*! \file kl02z_gpio.c
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */


#include "hwgpio.h"
#include <stdarg.h>
#include "debug.h"
#include "hwatomic.h"
#include "MKL02Z4.h"
#include "fsl_port_hal.h"
#include "fsl_gpio_hal.h"
#include "fsl_gpio_driver.h"

#define GPIO_BASE(port) g_gpioBase[port]
#define PORT_BASE(port) g_portBase[port]

typedef struct
{
    gpio_inthandler_t callback;
    int8_t pin;
    port_interrupt_config_t interrupt_config;
} gpio_interrupt_t;

static gpio_interrupt_t portb_interrupt_config[8];
static uint8_t portb_interrupts = 0x00;

__LINK_C void __gpio_init()
{
    for(int i = 0; i < 8; i++)
    {
        portb_interrupt_config[i].callback = 0x0;
        portb_interrupt_config[i].pin = 0xFF;
        portb_interrupt_config[i].interrupt_config = kPortIntDisabled;
    }
}

__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed, uint8_t mode, unsigned int out)
{
    // TODO
    return SUCCESS;
}

__LINK_C error_t hw_gpio_set(pin_id_t pin_id)
{
    GPIO_HAL_WritePinOutput(GPIO_BASE(pin_id.port), pin_id.pin, 1);
    return SUCCESS;
}

__LINK_C error_t hw_gpio_clr(pin_id_t pin_id)
{
    GPIO_HAL_WritePinOutput(GPIO_BASE(pin_id.port), pin_id.pin, 0);
    return SUCCESS;
}

__LINK_C error_t hw_gpio_toggle(pin_id_t pin_id)
{
    GPIO_HAL_TogglePinOutput(GPIO_BASE(pin_id.port), pin_id.pin);
    return SUCCESS;
}

__LINK_C bool hw_gpio_get_out(pin_id_t pin_id)
{
    // TODO
}

__LINK_C bool hw_gpio_get_in(pin_id_t pin_id)
{
    return GPIO_HAL_ReadPinInput(GPIO_BASE(pin_id.port), pin_id.pin);
}

void PORTB_IRQHandler()
{
    // TODO we only check for GDO0, hardcoded for now
    if(PORT_HAL_IsPinIntPending(PORTB, CC1101_GDO0_PIN.pin))
    {
        PORT_HAL_ClearPinIntFlag(PORTB, CC1101_GDO0_PIN.pin);
        if(portb_interrupt_config[CC1101_GDO0_PIN.pin].callback)
            portb_interrupt_config[CC1101_GDO0_PIN.pin].callback(CC1101_GDO0_PIN, 0); // TODO edge
    }
}

__LINK_C error_t hw_gpio_configure_interrupt(pin_id_t pin_id, gpio_inthandler_t callback, uint8_t event_mask)
{
    assert(PORT_BASE(pin_id.port) == PORTB); // TODO multiple ports not supported yet
    assert(callback != NULL);

    portb_interrupt_config[pin_id.pin].callback = callback;

    port_interrupt_config_t interrupt_config;
    if(event_mask == 0)
        interrupt_config = kPortIntDisabled;
    else if(event_mask & GPIO_FALLING_EDGE && event_mask & GPIO_RISING_EDGE)
        interrupt_config = kPortIntEitherEdge;
    else if(event_mask & GPIO_FALLING_EDGE)
        interrupt_config = kPortIntFallingEdge;
    else if(event_mask & GPIO_RISING_EDGE)
        interrupt_config = kPortIntRisingEdge;

    portb_interrupt_config[pin_id.pin].interrupt_config = interrupt_config;
    portb_interrupts &= 1 << pin_id.port;
    PORT_HAL_SetPinIntMode(PORT_BASE(pin_id.port), pin_id.pin, kPortIntDisabled);
    return SUCCESS;
}
__LINK_C error_t hw_gpio_enable_interrupt(pin_id_t pin_id)
{
    assert(portb_interrupt_config[pin_id.pin].callback != NULL);
    PORT_HAL_SetPinIntMode(PORT_BASE(pin_id.port), pin_id.pin, portb_interrupt_config[pin_id.pin].interrupt_config);
    return SUCCESS;
}

__LINK_C error_t hw_gpio_disable_interrupt(pin_id_t pin_id)
{
    PORT_HAL_SetPinIntMode(PORT_BASE(pin_id.port), pin_id.pin, kPortIntDisabled);
    return SUCCESS;
}
