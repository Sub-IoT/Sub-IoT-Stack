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

/*! \file cortus_gpio.c
 *
 *  \author junghoon 
 *
 */


#include "hwgpio.h"
#include <assert.h>
#include "hwatomic.h"
#include "machine/ic.h"
#include "machine/gpio.h"
#include "machine/capint.h"
#include <machine/cpu.h>

#define NUM_GPIOINT 16  // GPIO1 is only available for gpio interrupts.


// There is a limitation for using gpio interrupts. You can use gpio interrupts upto 16 pins in only gpio1 module(0 to 15).
static gpio_inthandler_t gpio_callback[NUM_GPIOINT];


__LINK_C void __gpio_init()
{
    // initialize interrupts[]
    for(int i = 0; i<NUM_GPIOINT; i++) gpio_callback[i] = 0x00;

    // enable interrupt for GPIO1
    irq[IRQ_GPIO_EDGE1].ipl = 0;
    irq[IRQ_GPIO_EDGE1].ien = 1;
    /* ic->ien = 1;// Don't forget this setting before running program */ 
}

__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed, uint8_t mode, unsigned int out)
{
    if((int_allowed) && (gpio_callback[pin_id.pin] != 0x00)) return EBUSY;

    GPio_edge *TGpio =   (GPio_edge*) pin_id.port;
    TGpio->dir  &=  (0x0ffffffff ^ (1 << pin_id.pin));
    TGpio->dir  |=  (mode << pin_id.pin);    // mode => 0:input 1:output
    TGpio->out  &=  (0x0ffffffff ^ (1 << pin_id.pin));
    TGpio->out  |=  (out << pin_id.pin);

    return SUCCESS;
}

__LINK_C error_t hw_gpio_set(pin_id_t pin_id)
{
	GPio_edge *TGpio =  (GPio_edge*) pin_id.port;
    TGpio->out  |= (1 << pin_id.pin);
    return SUCCESS;
}

__LINK_C error_t hw_gpio_clr(pin_id_t pin_id)
{
	GPio_edge *TGpio = (GPio_edge*) pin_id.port;
    TGpio->out &= (0x0ffffffff ^ (1 << pin_id.pin));
    return SUCCESS;
}

__LINK_C error_t hw_gpio_toggle(pin_id_t pin_id)
{
	GPio_edge *TGpio = (GPio_edge*) pin_id.port;
    TGpio->out ^= (1<<pin_id.pin);
    return SUCCESS;
}

__LINK_C bool hw_gpio_get_out(pin_id_t pin_id)
{
	GPio_edge *TGpio = (GPio_edge*) pin_id.port;
    return ((TGpio->out >> pin_id.pin)&1);
}

__LINK_C bool hw_gpio_get_in(pin_id_t pin_id)
{
	GPio_edge *TGpio = (GPio_edge*) pin_id.port;
    return ((TGpio->in >> pin_id.pin)&1);
}

void interrupt_handler(IRQ_GPIO_EDGE1)
{
    uint8_t event_mask = 0; // This is a dummy var for compatible outline of call function.
    uint32_t inreg, old_inreg, mask, i;
    GPio_edge *TGpio = (GPio_edge*) SFRADR_GPIO_EDGE1;
    inreg       = TGpio->in;
    old_inreg   = TGpio->old_in;
    mask        = TGpio->mask;
    pin_id_t tmp_pin_id;

    tmp_pin_id.port = SFRADR_GPIO_EDGE1;

    //DPRINT ("INT in %02x old %02x", inreg, old_inreg);

    for(i=0; i<NUM_GPIOINT; i++)
    {
        if((gpio_callback[i] != 0x00) && (mask & 0x01))
        {
            if( (inreg&0x01) != (old_inreg&0x01) )
            {
                tmp_pin_id.pin = i;
                gpio_callback[i](tmp_pin_id, event_mask);
                break;
            }
        }

        inreg       >>= 1;
        old_inreg   >>= 1;
        mask        >>= 1;
    }
}

__LINK_C error_t hw_gpio_configure_interrupt(pin_id_t pin_id, gpio_inthandler_t callback, uint8_t event_mask)
{
    if((pin_id.pin >= NUM_GPIOINT) || (gpio_callback[pin_id.pin] != 0x00)) return EINVAL;
   
    start_atomic();
    GPio_edge *gpio_edge        = (GPio_edge *) SFRADR_GPIO_EDGE1;
    gpio_callback[pin_id.pin]   = callback;
    gpio_edge->old_in           = gpio_edge->in;

    gpio_edge->edge = 0x1; // Clear all edges
    gpio_edge->level_sel |= (1<<pin_id.pin);// Select pin to interrupt

    if (event_mask == GPIO_RISING_EDGE)
        gpio_edge->rs_edge_sel = 0x1;
    else if (event_mask == GPIO_FALLING_EDGE)
        gpio_edge->fl_edge_sel = 0x1;
    else
    {
        end_atomic();
        return FAIL;
    }

    end_atomic();

    return SUCCESS;
}
__LINK_C error_t hw_gpio_enable_interrupt(pin_id_t pin_id)
{
    start_atomic();
    GPio_edge *gpio_edge = (GPio_edge*) SFRADR_GPIO_EDGE1;

    // update mask register
    gpio_edge->mask |= (1<<pin_id.pin);

    //DPRINT ("Enable: mask = %08x pin_id %04x in %02x", gpio_edge->mask, pin_id.pin, gpio_edge->in);

    end_atomic();

    return SUCCESS;
}

__LINK_C error_t hw_gpio_disable_interrupt(pin_id_t pin_id)
{
    start_atomic();
    GPio_edge *gpio_edge = (GPio_edge*) SFRADR_GPIO_EDGE1;

    // update mask register
    gpio_edge->mask  &= (0x0ffffffff ^ (1<<pin_id.pin));

    //DPRINT ("Disable: mask = %08x pin_id %04x in %02x", gpio_edge->mask, pin_id.pin, gpio_edge->in);

    end_atomic();

    return SUCCESS;
}

__LINK_C error_t hw_gpio_set_edge_interrupt(pin_id_t pin_id, uint8_t edge)
{
    GPio_edge *gpio_edge = (GPio_edge*) pin_id.port;

    start_atomic();
    gpio_edge->mask  &= (0x0ffffffff ^ (1<<pin_id.pin));

    gpio_edge->edge = 0x1; // Clear all edges
    gpio_edge->old_in = gpio_edge->in;
    gpio_edge->level_sel |= (1<<pin_id.pin); // Select pin to interrupt

    if (edge == GPIO_RISING_EDGE)
        gpio_edge->rs_edge_sel = 0x1;
    else
        gpio_edge->fl_edge_sel = 0x1;

    //DPRINT ("id %04x edge %d level_sel = %02x in %02x old %02x", pin_id.pin, edge, gpio_edge->level_sel, gpio_edge->in, gpio_edge->old_in);

    end_atomic();

    return SUCCESS;
}
