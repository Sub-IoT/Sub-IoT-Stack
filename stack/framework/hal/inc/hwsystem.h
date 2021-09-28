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

/*! \file hwsystem.h
 * \addtogroup System
 * \ingroup HAL
 * @{
 * \brief Definition of various platform-dependent 'system' functions
 */

#ifndef __HW_SYSTEM_H_
#define __HW_SYSTEM_H_

#include "types.h"
#include "link_c.h"

typedef enum {
    REBOOT_REASON_POR = 0,
    REBOOT_REASON_WDT = 1,
    REBOOT_REASON_SOFTWARE_REBOOT = 2,
    REBOOT_REASON_RESET_PIN = 3,
    REBOOT_REASON_OTHER = 254,
    REBOOT_REASON_NOT_IMPLEMENTED = 255,
} system_reboot_reason_t;

system_reboot_reason_t hw_system_reboot_reason(void);
void hw_system_save_reboot_reason(void);

/*! \brief Put the system in low power mode
 *
 * When the system is in low power mode the CPU is halted until it
 * receives an external trigger (usually an interrupt).
 *
 * The low power mode to enter is selected by supplying the mode
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
 * \param mode  The low power mode to enter. Defaults to 0
 *
 */
__LINK_C void hw_enter_lowpower_mode(uint8_t mode);


/** \brief Deinitializes all pheriperals before going to low power mode.
 * This is a weak symbol which can be implemented in the platform if you want to use this
 */
__LINK_C __attribute__((weak)) void hw_deinit_pheriperals(void);

/** \brief Reinitializes all pheriperals after resuming from low power mode.
 * This is a weak symbol which can be implemented in the platform if you want to use this
 */
__LINK_C __attribute__((weak)) void hw_reinit_pheriperals(void);

/*! \brief Get a 64-bit identifier that is unique to the device on which this function is called.
 *
 * The exact manner in which this ID is generated depends on the specific platform. In general however,
 * it is generated based on either the hardware MAC address of an attached network device or the serial 
 * number of the MCU.
 *
 * By convention the bytes of the ID should be ordered in such a way as to minimise the risk of 
 * a collision in the event that the ID is truncated.
 * 
 * If, for instance, the ID is generated based on an MCU specific part_number and a device specific 
 * serial_number, the bits of the serial_number should be placed in the least significant bits of the 
 * returned ID.
 *
 * \return uint64_t A device unique identifier
 */
__LINK_C uint64_t hw_get_unique_id(void);

/*! \brief Starts a busy wait loop blocking execution for the specified number of microseconds.
  *
  * Note: this should only be called from drivers.
  *
  * \param microseconds The number of microseconds to loop.
  */
__LINK_C void hw_busy_wait(int16_t microseconds);

/*! \brief Resets the MCU.
  */
__LINK_C void hw_reset(void);

/*!
 * /brief Returns the internal temperature of the MCU
 */
__LINK_C float hw_get_internal_temperature(void);

/*!
 * /brief Returns the battery voltage in mV (VDD)
 */
__LINK_C uint32_t hw_get_battery(void);

#endif //__HW_SYSTEM_H

/** @}*/
