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

/* \file
 *
 * Interface to the hardware buttons of the gecko board. This file is NOT a part of the
 * 'HAL' interface since not every platform will have buttons
 *
 * The EFM32GG 'Giant Geck' board has two userbuttons. PB0 and PB1.
 * These buttons are identified using a button_id_t.
 *  - PB0 is identified with button id '0'
 *  - PB1 is identified with button id '1'
 *
 */
#ifndef __PLATFORM_USERBUTTON_H_
#define __PLATFORM_USERBUTTON_H_

#include "link_c.h"
#include "types.h"
#include "platform.h"

/* \brief The identifiers for the buttons
 */
typedef uint8_t button_id_t;

/* \brief The callback function for when a button is pressed
 *
 * \param button_id		The id of the button that was pressed
 * **/
typedef void (*ubutton_callback_t)(button_id_t button_id);

/* \brief Check whether a button is currently pressed or not.
 *
 * if an invalid button_id is supplied, this function returns false
 *
 * \return bool		TRUE if the button is pressed
 * 					FALSE if the button is not pressed or if an invalid button id was supplied
 * */
__LINK_C bool ubutton_pressed(button_id_t button_id);

/* \brief Register a function to be called when the button with the specified <button_id> is pressed
 *
 * Multiple callback functions can be registered for the same button but the same function cannot be registered twice.
 * If a previously registered callback is re-registered for the same button, EALREADY is returned
 *
 *  \param	button_id	The id of the button for which to register the callback '0' for PB0, '1' for PB1
 *  \param	callback	The function to call when the button is pressed
 *  \return	error_t		SUCCESS if the callback was successfully registered
 *  					ESIZE if an invalid button_id was specified
 *  					EINVAL if callback is 0x0
 *						EALREADY if the callback was already registered for this button
 *						ENOMEM	if the callback could not be registered because there are already too many
 *								callbacks registered for this button
 */
__LINK_C error_t ubutton_register_callback(button_id_t button_id, ubutton_callback_t callback);

/* \brief Deregister a callback function previously registered using 'register_button_callback'
 *
 *  \param	button_id	The id of the button for which to register the callback '0' for PB0, '1' for PB1
 *  \param	callback	The function to deregister
 *  \return	error_t		SUCCESS if the callback was successfully deregistered
 *  					ESIZE if an invalid button_id was specified
 *  					EINVAL if callback is 0x0
 *						EALREADY if the callback was not registered for this button
 */
__LINK_C error_t ubutton_deregister_callback(button_id_t button_id, ubutton_callback_t callback);

//not a user function
void __ubutton_init();

#endif
