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

/*! \file hwusb.h
 * \addtogroup USB
 * \ingroup HAL
 * @{
 * \brief Provides an API for controlling USB
 * \author maarten.weyn@uantwerpen.be
 */

#ifndef __USB_H__
#define __USB_H__

#include "platform.h"

#include "types.h"
#include "link_c.h"

/*! \brief Initialise the USB communication class device of the platform
 *
 *  This function initialises the USB of the platform and actives the communicaiton class device. This function is NOT part of the
 *  'user' API and should only be called from the initialisation code of the specific platform
 *
 */
__LINK_C void __usb_init_cdc();

#endif // __USB_H__

/** @}*/
