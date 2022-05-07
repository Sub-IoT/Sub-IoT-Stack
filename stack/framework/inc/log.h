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

/*! \file log.h
 * \addtogroup logging
 * \ingroup framework
 * @{
 * \brief Specifies the generic logging facilities of the framework.
 *
 * A number of functions are provided to log both arbitrarily formatted data
 * (processed using the printf family of functions) and binary data (logged in a 
 * more or less human readable format).
 *
 * Logging can be globally enabled or disabled by setting or clearing the 
 * 'FRAMEWORK_LOG_ENABLED' CMake option.
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

/*! \brief The source in the stack from which the log originates  */
typedef enum
{
    LOG_STACK_PHY = 0x01,
    LOG_STACK_DLL = 0x02,
    LOG_STACK_MAC = 0x03,
    LOG_STACK_NWL = 0x04,
    LOG_STACK_TRANS = 0x05,
    LOG_STACK_SESSION = 0x06,
    LOG_STACK_D7AP = 0x07,
    LOG_STACK_ALP = 0x08,
    LOG_STACK_FWK = 0x10,
    LOG_STACK_EM = 0x11
} log_stack_layer_t; // TODO stack specific, move to stack component?

#ifdef FRAMEWORK_LOG_ENABLED

/*! \brief Reset the log counter back to zero */
__LINK_C void log_counter_reset(void);

/*! \brief Log a string which can be optionally formatted using printf() style
 * format specifiers. */
__LINK_C void log_print_string(const char* format,...);

/*! \brief Log a string from a specific stack layer, which can be optionally formatted using printf() style
 * format specifiers. Note: this is only to be used from within stack code, not from application level code. */
__LINK_C void log_print_stack_string(log_stack_layer_t type, const char* format, ...);

/*! \brief Log raw data */
__LINK_C void log_print_data(uint8_t* message, uint32_t length);

/*! \brief Log a string using a bright red background to note an error */
void log_print_error_string(const char* format,...);

#else
    #define log_counter_reset() ((void)0)
    #define log_print_string(...) ((void)0)
    #define log_print_stack_string(...) ((void)0)
    #define log_print_data(...) ((void)0)
    #define log_print_error_string(...) ((void)0)
#endif

#endif /* __LOG_H_ */

/** @}*/
