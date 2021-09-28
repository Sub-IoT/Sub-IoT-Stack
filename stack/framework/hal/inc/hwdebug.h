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

/*! \file
 * \addtogroup debug
 * \ingroup HAL
 * @{
 * \brief The interface to the debug pins of the device. 
 *
 * A debug pin is a GPIO pin that has been configured
 * as an output pin and whose state (up/down) can be controlled through this interface.
 *
 * The application is able to signal specific events through this interface which can then be captured
 * through the use of an external logic analyzer or oscilloscope.
 *
 * The amount of debug pins available, and their wiring to external pins on the board itself,
 * depends on the specific hardware platform used. A platform is not required to provide any hardware
 * debugging capabilities, in which case calls to the functions defined in this file are ignored.
 *
 */
#ifndef HWDEBUG_H_
#define HWDEBUG_H_

#include "types.h"
#include "link_c.h"
#include "platform_defs.h"

#if PLATFORM_NUM_DEBUGPINS > 0
    #define DEBUG_PIN_SET(pin) hw_debug_set(pin)
    #define DEBUG_PIN_CLR(pin) hw_debug_clr(pin)
    #define DEBUG_PIN_TOGGLE(pin) hw_debug_toggle(pin)
#else
    #define DEBUG_PIN_SET(pin) ((void)0)
    #define DEBUG_PIN_CLR(pin) ((void)0)
    #define DEBUG_PIN_TOGGLE(pin) ((void)0)
#endif

/*! \brief Initialise the debug pins of the platform
 *
 *  This function initialises the debug pins of the platform. This function is NOT part of the
 *  'user' API and should only be called from the initialisation code of the specific platform
 *
 */
__LINK_C void __hw_debug_init();

/*! \brief Set the specified debug pin.
 *
 *  If a debug pin is set, the output of the corresponding GPIO pin is pulled HIGH.
 *  If an unsupported pin_id is specified this call MUST be ignored by the implementation
 *
 *  \param pin_id	The id of the pin to set
 */
__LINK_C void hw_debug_set(uint8_t pin_id);

/*! \brief Clear the specified debug pin.
 *
 *  If a debug pin is set, the output of the corresponding GPIO pin is pulled LOW.
 *  If an unsupported pin_id is specified this call MUST be ignored by the implementation
 *
 *  \param pin_id	The id of the pin to clear
 */
__LINK_C void hw_debug_clr(uint8_t pin_id);

/*! \brief Toggle the state of the specified debug pin.
 *
 *  If the pin was set it is now cleared and vice versa.
 *  If an unsupported pin_id is specified this call MUST be ignored by the implementation
 *
 *  \param pin_id	The id of the pin to clear
 */
__LINK_C void hw_debug_toggle(uint8_t pin_id);

/*! \brief Set/Clear multiple debug pins at the same time
 *
 * This function accepts a 'mask' of debug pins to set or clear. Each bit in the mask represents
 * a single debug pin. More specifically debug pin 'i' is mapped to the ith bit in the mask (counting from
 * least to most significant). When the bit corresponding to a bit is '1', the pin is set. Of the bit corresponding
 * to a pin is '0' it is cleared.
 *
 * On platforms that allow multiple GPIO pins to be configured with a single instruction, all the pins in the mask are
 * set/cleared at the same time. If this is not supported, the pins are configured iteratively
 *
 * \param mask	The mask of debug pins to set/clear. Each bit in the specified mask represent
 *
 */
__LINK_C void hw_debug_mask(uint32_t mask);

#endif /* HWDEBUG_H_ */

/** @}*/
