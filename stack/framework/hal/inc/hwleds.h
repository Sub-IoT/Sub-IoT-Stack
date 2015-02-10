/*! \file leds.h
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
 * \author maarten.weyn@uantwerpen.be
 * \author glenn.ergeerts@uantwerpen.be
 * \author daniel.vandenakker@uantwerpen.be
 *
 */

#ifndef __LEDS_H__
#define __LEDS_H__

#include "platform.h"

#ifndef HW_NUM_LEDS
    #warning the platform does not define HW_NUM_LEDS
#endif

#include "types.h"


/*! \brief Initialise the leds of the platform
 *
 *  This function initialises the leds of the platform. This function is NOT part of the
 *  'user' API and should only be called from the initialisation code of the specific platform
 *
 */
void __led_init();

/*! \brief Turn a led on
 *
 *  \param led_id The id of the led to turn on. This must be a number between 0
 * 		  and HW_NUM_LEDS. Out-of-range indexes are silently ignored
 *
 */
void led_on(uint8_t led_id);

/*! \brief Turn a led off
 *
 *  \param led_id The id of the led to turn off. This must be a number between 0
 * 		  and HW_NUM_LEDS. Out-of-range indexes are silently ignored
 *
 */
void led_off(uint8_t led_id);

/*! \brief Toggle a led. If it was on it is turned off and vice versa.
 *
 *  \param led_id The id of the led to toggle. This must be a number between 0
 * 		  and HW_NUM_LEDS. Out-of-range indexes are silently ignored
 *
 */
void led_toggle(uint8_t led_nr);

//may be nice to have, but doesn't belong in the HAL (it's only used from applications -> can be dumped in a 
//separate module if needed
//void led_blink(uint8_t led_id);

#endif // __LEDS_H__
