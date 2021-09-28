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

#ifndef __EZR32WG_CHIP_H_
#define __EZR32WG_CHIP_H_
#include "hwgpio.h"

#define PLATFORM_NUM_TIMERS 1

#define GPIO_PIN(pin_id) (pin_id & 0x0F)
#define GPIO_PORT(pin_id) ((pin_id >> 4) & 0x0F)

/* \brief Implementation of hw_gpio_configure_pin for the EZR32WG MCU
 *
 * This function supports all pin configuration modes available through the
 * 'GPIO_PinModeSet' function in emlib.
 *
 * It should be noted that the EZR32WG only has a single interrupt bit for each pin id
 * and that only a single GPIO port can be selected for each pin id. This means that it is,
 * for instance not possible to configure an interrupt both for in A3 and D3 since these pins share
 * the same pin id (3 in this case) but are wired on different ports. A3 and D4 do not conflict
 * because they have a different pin id.
 *
 * Because of this, this function also requires the user to explicitly choose whether or not
 * interrupts can be enabled for a specific pin through the <int_allowed> parameter.
 * If <int_allowed> is true for the given pin, the interrupts can later be configured
 * through 'hw_gpio_configure_interrupt'. Otherwise, 'hw_gpio_configure_interrupt' will
 * always return EOFF for the given pin.
 *
 * When this function is called with <int_allowed> set to true, after interrupts have been
 * explicitly allowed for a conflicting pin, this function will return EBUSY and the gpio port
 * will not be configured.
 *
 * \param pin_id(pin_id_t)		The pin_id of the GPIO pin to configure
 * \param int_allowed(bool)		Whether or not to allow interrupts for this pin
 * \param mode(GPIO_Mode_TypeDef)	The mode for the GPIO pin (see EZR32WG docs for more information)
 * \param out(unsigned int)		The out value for the GPIO pin (see EZR32WG docs for more information)
 *
 * \return	SUCCESS if the pin was configured successfully
 *		EBUSY if <int_allowed> is true and interrupts were already allowed for a conflicting pin
 */
__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed, uint8_t mode, unsigned int out);



#endif //__EZR32WG_CHIP_H_
