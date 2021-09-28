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

/*! \file hwwatchdog.h
 * \addtogroup watchdog
 * \ingroup HAL
 * @{
 * \brief Watchdow API
 * \author glenn.ergeerts@uantwerpen.be
 */

#ifndef __HWWATCHDOG_H__
#define __HWWATCHDOG_H__

#include "types.h"
#include "link_c.h"

/*! \brief Initialise the watchdog
 *
 *  This function initialises the watchdog of the selected MCU. This function is NOT part of the
 *  'user' HAL API and should only be called from the initialisation code of the specific platform.
 *  The configuration of the watchdog is hardcoded in the platform (for now).
 *  After calling this function the watchdog timer is enabled and should be resetted periodically using hw_watchdog_feed().
 *  Failing to do so before the timer elapses results in MCU reset.
 *
 */
__LINK_C void __watchdog_init(void);

/*! \brief Feeds the watchdog.
 */
__LINK_C void hw_watchdog_feed(void);

/*! \brief Gets the timeout value of the watchdog timer in seconds.
 */
__LINK_C uint8_t hw_watchdog_get_timeout(void);

#endif // __HWWATCHDOG_H__

/** @}*/
