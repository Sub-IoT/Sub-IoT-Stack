/*! \file
 *
 * Definition of various platform-dependent 'system' functions
 *
 */

#ifndef __HW_SYSTEM_H_
#define __HW_SYSTEM_H_

#include "types.h"
#include "link_c.h"

/*! \brief Put the system in low power mode
 *
 * When the system is in low power mode the CPU is halted until it
 * receives an external trigger (usually an interrupt).
 *
 * The low power mode to enter is selected by supplying the <mode>
 * parameter. 
 *
 * In low power mode zero (mode == 0) the CPU itself is disabled but all 
 * hardware peripherals are required to remain available. (This usually corresponds to 
 * the most power hungry low power mode of the MCU).
 *
 * The HAL is only required to support (mode == 0) but other low power modes (mode > 0) 
 * may be available, depending on the specific platform. In general these additional 
 * low power modes trade energy usage for the (un)availability of certain peripherals. 
 * By convention, higher values for the 'mode' parameter select lower power modes. 
 * The exact definition of each additional low power mode however, depends on the 
 * specific platform.
 *
 * \param mode	The low power mode to enter. Defaults to 0
 *
 */
__LINK_C void hw_enter_lowpower_mode(uint8_t mode);

/*! \brief Get a 64-bit identifier that is unique to the device on which this function is called.
 *
 * The exact manner in which this ID is generated depends on the specific platform. In general however,
 * it is generated based on either the hardware MAC address of an attached network device or the serial 
 * number of the MCU.
 *
 * By convention the bytes of the ID should be ordered in such a way as to minimise the risk of 
 * a collision in the event that the ID is truncated.
 * 
 * If, for instance, the ID is generated based on an MCU specific <part_number> and a device specific 
 * <serial_number>, the bits of the <serial_number> should be placed in the least significant bits of the 
 * returned ID.
 *
 * \return uint64_t	A device unique identifier
 */
__LINK_C uint64_t hw_get_unique_id();

#endif //__HW_SYSTEM_H
