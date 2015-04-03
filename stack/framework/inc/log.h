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
 
 * This file specifies the generic logging facilities of the framework.
 * A number of functions are provided to log both arbitrarily formatted data
 * (processed using the printf family of functions) and binary data (logged in a 
 * more or less human readable format). Moreover data can be logged in human 
 * readable format or in binary format. Which format is used is controlled through the 
 * 'LOG_BINARY' CMake option.
 * 
 * Logging can be globally enabled or disabled by setting or clearing the 
 * 'LOGGING_ENABLED' CMake option. Moreover
 *
 * \author maarten.weyn@uantwerpen.be
 * \author glenn.ergeerts@uantwerpen.be
 * \author daniel.vandenakker@uantwerpen.be
 *
 */
#ifndef __LOG_H_
#define __LOG_H_

#include "link_c.h"
#include "framework_defs.h"
#include "types.h"

#ifdef FRAMEWORK_LOG_ENABLED

typedef enum
{
    LOG_STACK_PHY = 0x01,
    LOG_STACK_DLL = 0x02,
    LOG_STACK_MAC = 0x03,
    LOG_STACK_NWL = 0x04,
    LOG_STACK_TRANS = 0x05,
    LOG_STACK_FWK = 0x10
} log_stack_layer_t; // TODO stack specific, move to stack component?

/* \brief Reset the log counter back to zero
 *
 */
__LINK_C void log_counter_reset();

__LINK_C void log_print_string(char* format,...);
__LINK_C void log_print_stack_string(log_stack_layer_t type, char* format, ...);
__LINK_C void log_print_data(uint8_t* message, uint32_t length);

#else

//we use static inline replacements instead of 'defining them away'
//to ensure that side-effects resulting from the evaluation of the parameters
//are still performed
__LINK_C static inline void log_counter_reset() {}
__LINK_C static inline void log_print_string(char* format,...) {}
__LINK_C static inline void log_print_stack_string(char type, char* format, ...) {}
__LINK_C static inline void log_print_data(uint8_t* message, uint8_t length) {}


#endif

#endif /* __LOG_H_ */
