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
 *	\addtogroup GPIO
 * 	\ingroup HAL
 * @{
 * \brief Hardware interface to the GPIO Pins of the MCU of the platform.
 *
 * This header files specifies a number of generic functions for accessing 
 * and configuring individual General Purpose IO pins of the MCU.
 *
 * The GPIO interface allows different pieces of code to interact with 
 * the MCU's GPIO pins in different manners.
 *
 * This interface allows:
 *   - The MCU to provide a generic, platform independent, interface 
 *     to the GPIO pins
 *
 *   - Chip drivers / 'high level' users to:
 *       - Get/Set/Clear specific IO pins
 *       - Configure callbacks to be called for specific GPIO events
 *     In an MCU/Platform independent manner
 *
 *   - Platform integrators to:
 *	 - Decide which pins are available
 *	 - Wire specific IO pins to specific peripherals
 *	 - Configure individual pins as input / output
 *	 - Do 'advanced' PIN configuration on MCU's that support it
 *	   (such as the EFM32gg)
 *
 * Each GPIO pin of the MCU is identified with a unique 'pin_id_t'.
 * The pin_id's for the GPIO pins are defined by the MCU and made available
 * to chip drivers & user functions through the platform-specific "platform.h"
 * header file. These pin_id's can then be passed to the functions defined below
 * to interact with individual GPIO pins.
 * 
 */
#ifndef __HW_GPIO_H_
#define __HW_GPIO_H_
#include "types.h"
#include "link_c.h"
/*! \brief the type for the pin_id's that identify each individual GPIO pin
 *
 */
typedef uint32_t pin_id_t;

enum
{
    /*! \brief value in the event_mask to signify a transition from low to high
     */
    GPIO_RISING_EDGE = 1,
    /*! \brief value in the event_mask to signify a transition from high to low
     */
    GPIO_FALLING_EDGE = 2,
};

/*! \brief Generic callback function for GPIO events.
 *
 * gpio_inthander's are callback functions that are called when specific events occur
 * for specific IO pins.
 * 
 * A callback can be configured by calling 'hw_gpio_configure_interrupt()'. 
 * Once the configured event occurs, the callback is called. Please note that callbacks 
 * are called in interrupt context and that any processing therefore needs to be as minimal
 * as possible.
 *
 * \param arg    Optional argument for the callback
 */
typedef void (*gpio_cb_t)(void *arg);

/**
 * @brief   Default interrupt context for GPIO pins
 */
typedef struct {
	gpio_cb_t cb;           /**< interrupt callback */
    void *arg;              /**< optional argument */
} gpio_isr_ctx_t;

/*! \brief Set the output of a GPIO pin to 'high'
 * 
 * It should be noted that only pins that have previously been configured
 * for 'output' can be set 'high'. If the pin was not configured,
 * EOFF is returned and the state of the pin remains unchanged
 *
 * \param pin_id 	The id of the GPIO pin to set 'high'
 * \return error_t	SUCCESS if the pin was successfully set 'high'
 *			EOFF if the pin was not configured for output.
 */
__LINK_C error_t hw_gpio_set(pin_id_t pin_id);

/*! \brief Set the output of a GPIO pin to 'low'
 * 
 * It should be noted that only pins that have previously been configured
 * for 'output' can be set 'low'. If the pin was not configured,
 * EOFF is returned and the state of the pin remains unchanged
 *
 * \param pin_id 	The id of the GPIO pin to set 'low'
 * \return error_t	SUCCESS if the pin was successfully set 'low'
 *			EOFF if the pin was not configured for output.
 */
__LINK_C error_t hw_gpio_clr(pin_id_t pin_id);

/*! \brief Toggle the output of a GPIO pin. If the pin was previously 
 *	  configured as 'high' it is now configured 'low' and vice versa.
 * 
 * It should be noted that only pins that have previously been configured
 * for 'output' can be toggled. If the pin was not configured,
 * EOFF is returned and the state of the pin remains unchanged
 *
 * \param pin_id 	The id of the GPIO pin to toggle
 * \return error_t	SUCCESS if the pin was successfully toggled
 *			EOFF if the pin was not configured for output.
 */
 __LINK_C error_t hw_gpio_toggle(pin_id_t pin_id);

/*! \brief Retrieve the current output 'state' of the GPIO pin
 *
 * This function returns true or false depending on whether the 'output'
 * state of the GPIO pin is currently high or low. For pins that have been configured as
 * 'output' pins, this reflects the state of the pin configured with
 * hw_gpio_set() and hw_gpio_clr(). 
 * For 'input' pins, this value is undefined.
 the state of the pin is depends on the exact manner in which the 
 * pin was configured,  * (see hw_gpio_configure_pin() for more information), and whether or not it
 * is currently being pulled high or low by an external peripheral.
 *
 * \param pin_id	The id of the pin to query
 * \returns bool	true if the pin is currently high
 *			false if the pin is currently low
 *			or not configured for use.
 */
__LINK_C bool hw_gpio_get_out(pin_id_t pin_id);

/*! \brief Retrieve the current input 'state' of the GPIO pin
 *
 * This function returns true or false depending on whether the 'input'
 * state of the GPIO pin is currently high or low. For pins that have been configured as
 * 'input' pins, this reflects whether the pin is currently being pulled high or low
 * by an external peripheral. 
 *
 * \param pin_id	The id of the pin to query
 * \returns bool	true if the pin is currently high
 *			false if the pin is currently low
 *			or not configured for use.
 */
__LINK_C bool hw_gpio_get_in(pin_id_t pin_id);

/*! \brief Configure an interrupt to occur when a rising or falling event occurs
 *	  on a specific GPIO pin.
 *
 * GPIO interrupts cannot be configured for all GPIO pins. Whether or not a GPIO
 * interrupt can be configured depends on how the pin was configured by the platform
 * (ie: the call to hw_gpio_configure_pin). In general however, a GPIO interrupt can only be
 * configured for GPIO pins that have been configured as 'input' pins. When this function
 * is called for a GPIO pin that does not support interrupts, this is signalled to the caller
 * by returning EOFF.
 *
 * Interrupts can be scheduled both for 'rising edge' and 'falling edge' events. Although 
 * this function allows both 'rising' and 'falling' events to be captured for the same 
 * GPIO pin at the same time, it should  be noted that this is not supported by all 
 * MCU's (the msp430 for instance does not support this). If a GPIO interrupt is configured
 * for both a rising and a falling event while this is not supported, EINVAL is returned.
 *
 * This function can be called multiple times to alter the event_mask of the events to capture.
 * The callback function however MUST match the function of the first call to this function (for 
 * a specific GPIO pin). If the callback function doesn't match, EBUSY is returned.
 *
 * Please note that configuring the event does not automatically enable it. The interrupt must
 * be enabled through a separate call to hw_gpio_enable_interrupt. If the interrupt was 
 * previously enabled by a call to hw_gpio_enable_interrupt, it is disabled by a call to 
 * this function.
 *
 * \param pin_id	The id of the GPIO pin on which to configure the interrupt
 * \param event_mask	A bitmask of the events that should be captured. The event_mask can
 *			be one of three values:
 *			  - 0: no events will be captured
 *			  - GPIO_RISING_EDGE: capture rising edge events only
 *			  - GPIO_FALLING_EDGE: capture falling edge events only
 *			  - (GPIO_RISING_EDGE | GPIO_FALLING_EDGE): capture both
 *				rising and falling edge events.
 *	\param callback	The function to call when the interrupt occurs. Please note that
 *			this function is called from an interrupt context and that all processing
 *			should therefore be as minimal as possible.
 *	\param arg The parameter to pass to the callback function.
 *	\return error_t	SUCCESS if the GPIO interrupt was configured successfully
 *			EOFF if the selected GPIO pin does not support GPIO interrupts
 *			EINVALID if the MCU does not support the event_mask specified
 *			         or if the supplied callback is 0x0
 *			EBUSY if the callback function does not match the callback
 *			      function of an earlier call to this function
 */
__LINK_C error_t hw_gpio_configure_interrupt(pin_id_t pin_id, uint8_t event_mask,
                                             gpio_cb_t callback, void *arg);
/*! \brief Enable the interrupts for a specific GPIO pin.
 *
 * Please note that hw_gpio_configure_interrupt() must be called before the interrupt
 * can be enabled or disabled. Otherwise EOFF is returned and the interrupt remains disabled
 * 
 * The corresponding interrupt flag will be cleared to ensure it will not interrupt immediately if
 * this flag happens to be set before enabling the interrupt.
 *
 * \param pin_id	The GPIO pin for which to enable the interrupt
 * \return error_t	SUCCESS if the interrupt was enabled successfully
 *			EOFF if the interrupt has not yet been configured
 **/
__LINK_C error_t hw_gpio_enable_interrupt(pin_id_t pin_id);

/*! \brief Disable the interrupts for a specific GPIO pin.
 *
 * Please note that hw_gpio_configure_interrupt() must be called before the interrupt
 * can be enabled or disabled. Otherwise EOFF is returned and the interrupt remains disabled
 * 
 * \param pin_id	The GPIO pin for which to disable the interrupt
 * \return error_t	SUCCESS if the interrupt was disabled successfully
 *			EOFF if the interrupt has not yet been configured
 **/
__LINK_C error_t hw_gpio_disable_interrupt(pin_id_t pin_id);

/*! \brief Set the interrupt edge for a specific GPIO pin.
 *
 * \param pin_id	The GPIO pin for which to set the interrupt edge
 * \param edge      A bitmask of the events that should be captured. The event_mask can
 *			        be one of three values:
 *			  - 0: no events will be captured
 *			  - GPIO_RISING_EDGE: capture rising edge events only
 *			  - GPIO_FALLING_EDGE: capture falling edge events only
 *			  - (GPIO_RISING_EDGE | GPIO_FALLING_EDGE): capture both rising and falling edge events.
 * \return error_t	SUCCESS if the interrupt was set successfully
 **/
__LINK_C error_t hw_gpio_set_edge_interrupt(pin_id_t pin_id, uint8_t edge);

/*! \brief Configure a specific GPIO pin of the MCU for use
 *
 * This function is mainly intended to allow platform-specific initialization code to
 * configure the GPIO pins of the MCU for use by chip drivers and other modules.
 * If needed, it can also be used from application code to enable additional GPIO pins
 * at run-time, but this is extremely discouraged.
 *
 * In general, a GPIO pin can be configured for 'input', 'output' or 'both', but much 
 * more advanced configurations are also possible. Since the various  options available 
 * for configuring a GPIO pin are inherently tied the used MCU, and the hardware platform
 * on which the MCU resides, it is impossible to configure the GPIO pins without prior 
 * knowledge of which MCU is used.
 *
 * Because of this, this function is not declared here but must be instead declared in one
 * of the 'public' headers of the MCU used. See the documentation of this function for the
 * specific MCU used for more information.
 *
 */
//NOT declared here on purpose: actual declaration is included through "platform.h"
//__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, ...);

//not a user function: call this before initialising the individual GPIO pins
__LINK_C void __gpio_init();

#endif //__HW_GPIO_H_

/** @}*/
