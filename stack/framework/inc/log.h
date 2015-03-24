/*! \file log.h
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *     		glenn.ergeerts@uantwerpen.be
 *		daniel.vandenakker@uantwerpen.be
 *
 */

/* \file
 *
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
