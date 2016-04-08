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

/*! \file efm32hg_gpio.c
 *
 *  \author daniel.vandenakker@uantwerpen.be
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author contact@christophe.vg
 *
 */

#include <stdint.h>
#include <debug.h>

#include "em_gpio.h"
#include "em_cmu.h"
#include "em_bitband.h"

#include "hwgpio.h"
#include "hwatomic.h"

#include "gpiointerrupt.h"

#include "efm32hg_chip.h"

// interrupt callbacks : [gpioPortA(0)-gpioPortH(7)][0-15] = 8*16 = 128
static gpio_inthandler_t interrupts[128] = { NULL };
#define pin2index(p) ((p.port * 16) + p.pin)

// for each port (8) a bitfield with 16 bits represents all pins
static uint16_t gpio_pins_configured[8] = { 0 };
#define is_configured(p) (gpio_pins_configured[p.port] & (1<<p.pin))

__LINK_C void __gpio_init() {
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Initialize GPIO interrupt dispatcher
  GPIOINT_Init();
}

__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed,
                                       uint8_t mode, unsigned int out)
{
  // notify if pin was already configured, don't reconfigure
  if(is_configured(pin_id)) {	return EALREADY; }

  // mark the pin to as configured
  gpio_pins_configured[pin_id.port] |= (1<<pin_id.pin);
  
  // configure the pin itself
  GPIO_PinModeSet(pin_id.port, pin_id.pin, mode, out);
  
  return SUCCESS;    
}

__LINK_C error_t hw_gpio_set(pin_id_t pin_id) {
  if( ! is_configured(pin_id) ) { return EOFF; }
  GPIO_PinOutSet(pin_id.port, pin_id.pin);
  return SUCCESS;
}

__LINK_C error_t hw_gpio_clr(pin_id_t pin_id) {
  if( ! is_configured(pin_id) ) { return EOFF; }
  GPIO_PinOutClear(pin_id.port, pin_id.pin);
  return SUCCESS;
}

__LINK_C error_t hw_gpio_toggle(pin_id_t pin_id) {
  if( ! is_configured(pin_id) ) { return EOFF; }
  GPIO_PinOutToggle(pin_id.port, pin_id.pin);
  return SUCCESS;
}

__LINK_C bool hw_gpio_get_out(pin_id_t pin_id) {
  return is_configured(pin_id) && GPIO_PinOutGet(pin_id.port, pin_id.pin);
}

__LINK_C bool hw_gpio_get_in(pin_id_t pin_id) {
  return is_configured(pin_id) && GPIO_PinInGet(pin_id.port, pin_id.pin);
}

static void gpio_int_callback(uint8_t index) {
  // we use emlib's GPIO interrupt handler which does NOT disable the interrupts
  // by default --> disable them here to get the same behavior !!
  start_atomic();
  assert(interrupts[index] != NULL);
	pin_id_t pin = { port: index / 16, pin: index % 16 };
  interrupts[index](pin, 0);
  end_atomic();
}

__LINK_C error_t hw_gpio_configure_interrupt(pin_id_t pin_id,
                                             gpio_inthandler_t callback,
                                             uint8_t event_mask)
{
  if(callback == NULL || event_mask > (GPIO_RISING_EDGE | GPIO_FALLING_EDGE)) {
  	return EINVAL;
  }

  error_t err;
  start_atomic();
  // do this check atomically: interrupts[..] callback is altered by this
  // function. so the check belongs in the critical section as well
  if(interrupts[pin2index(pin_id)] == callback) {
    // we already have this one, shouldn't we return EALREADY ?
    // keep existing behaviour, success, and don't change anyting
    // err = EALREADY
    err = SUCCESS;
  } else if(interrupts[pin2index(pin_id)] != NULL ) {
    // whoops, already configured with another callback
    err = EBUSY;
  }	else {
    // configure the interrupt handler
    interrupts[pin2index(pin_id)] = callback;
  	GPIOINT_CallbackRegister(pin2index(pin_id), &gpio_int_callback);
    GPIO_IntConfig(
      pin_id.port, pin_id.pin, 
		  event_mask & GPIO_RISING_EDGE,
		  event_mask & GPIO_FALLING_EDGE,
		  false
    );			
    err = SUCCESS;
	}
  end_atomic();
  return err;
}

__LINK_C error_t hw_gpio_enable_interrupt(pin_id_t pin_id) {
  if(interrupts[pin2index(pin_id)] == NULL) {	return EOFF; }
  BITBAND_Peripheral(&(GPIO->IFC), pin_id.pin, 1);
  BITBAND_Peripheral(&(GPIO->IEN), pin_id.pin, 1);
  return SUCCESS;
}

__LINK_C error_t hw_gpio_disable_interrupt(pin_id_t pin_id)
{
  if(interrupts[pin2index(pin_id)] == NULL) { return EOFF; }
  BITBAND_Peripheral(&(GPIO->IEN), pin_id.pin, 0);
  return SUCCESS;
}
