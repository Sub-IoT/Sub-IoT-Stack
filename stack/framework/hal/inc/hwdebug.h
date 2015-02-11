/*! \file
 *
 * This file specifies the interface to the debug pins of the device. A debug pin is a GPIO pin that has been configured
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

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


/*! \brief Initialise the debug pins of the platform
 *
 *  This function initialises the debuh pins of the platform. This function is NOT part of the
 *  'user' API and should only be called from the initialisation code of the specific platform
 *
 */
void __hw_debug_init();

/*! \brief Set the specified debug pin.
 *
 *  If a debug pin is set, the output of the corresponding GPIO pin is pulled HIGH.
 *  If an unsupported pin_id is specified this call MUST be ignored by the implementation
 *
 *  \param pin_id	The id of the pin to set
 */
void hw_debug_set(uint8_t pin_id);

/*! \brief Clear the specified debug pin.
 *
 *  If a debug pin is set, the output of the corresponding GPIO pin is pulled LOW.
 *  If an unsupported pin_id is specified this call MUST be ignored by the implementation
 *
 *  \param pin_id	The id of the pin to clear
 */
void hw_debug_clr(uint8_t pin_id);

/*! \brief Toggle the state of the specified debug pin.
 *
 *  If the pin was set it is now cleared and vice versa.
 *  If an unsupported pin_id is specified this call MUST be ignored by the implementation
 *
 *  \param pin_id	The id of the pin to clear
 */
void hw_debug_toggle(uint8_t pin_id);

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
void hw_debug_mask(uint32_t mask);

#ifdef __cplusplus
}
#endif //__cplusplus


#endif /* HWDEBUG_H_ */
