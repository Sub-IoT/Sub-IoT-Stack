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

/*! \file hwradio.h
 * \addtogroup radio
 * \ingroup HAL
 * @{
 * \brief The interface specification for accessing the radio interface.
 *
 * \author Daniel van den Akker
 * \author Glenn Ergeerts
 * \author philippe.nunes@cortus.com
 *
 */
#ifndef __HW_RADIO_H_
#define __HW_RADIO_H_

#include "platform_defs.h"

typedef struct xcvr_handle xcvr_handle_t;

/**
 * @brief   Reference to the transceiver handle struct
 */
extern xcvr_handle_t xcvr;
#endif //__HW_RADIO_H_

/** @}*/
