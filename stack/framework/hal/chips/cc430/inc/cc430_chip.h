#ifndef __CC430_CHIP_H
#define __CC430_CHIP_H

#include "cc430_pins.h"

#define PLATFORM_NUM_TIMERS 1

typedef enum
{
    GPIO_MODE_OUTPUT,
    GPIO_MODE_OUTPUT_FULL_DRIVE_STRENGTH,
    GPIO_MODE_INPUT,
    GPIO_MODE_INPUT_PULL_UP,
    GPIO_MODE_INPUT_PULL_DOWN,
} cc430_gpio_mode_t;

/* \brief Implementation of hw_gpio_configure_pin for the cc430 MCU
 *
 * This function supports all pin configuration modes available through the
 * 'GPIO_x()' functions in TI driverlib supported for cc430f513x.
 *
 * \param pin_id(pin_id_t)		The pin_id of the GPIO pin to configure
 * \param mode(cc430_gpio_mode_t)	The mode for the GPIO pin
 *
 * \return	SUCCESS if the pin was configured successfully
 */
__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, cc430_gpio_mode_t mode);

#endif //__CC430_CHIP_H
