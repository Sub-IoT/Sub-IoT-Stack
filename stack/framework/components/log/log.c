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

/*! \file log.c
 * \author maarten.weyn@uantwerpen.be
 * \author glenn.ergeerts@uantwerpen.be
 * \author daniel.vandenakker@uantwerpen.be
 */

#include "log.h"
#include "string.h"
#include "ng.h"
#include "hwuart.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <hwradio.h>
#include "framework_defs.h"
#include "hwsystem.h"

#include "console.h"

#ifdef FRAMEWORK_LOG_ENABLED


static const uint16_t microsec_byte = 2*8000000/CONSOLE_BAUDRATE;
static uint32_t NGDEF(counter);


__LINK_C void log_counter_reset()
{
	NG(counter) = 0;
}

__LINK_C void log_print_string(char* format, ...)
{
    va_list args;
    va_start(args, format);
    printf("\n\r[%03d] ", NG(counter)++);
    vprintf(format, args);
    va_end(args);
    hw_busy_wait(microsec_byte);
}

__LINK_C void log_print_stack_string(log_stack_layer_t type, char* format, ...)
{
    va_list args;
    va_start(args, format);
    printf("\n\r[%03d] ", NG(counter)++);
    vprintf(format, args);
    va_end(args);
    hw_busy_wait(microsec_byte);
}

__LINK_C void log_print_data(uint8_t* message, uint32_t length)
{
    printf("\n\r[%03d]", NG(counter)++);
    for( uint32_t i=0 ; i<length ; i++ )
    {
        printf(" %02X", message[i]);
    }

    hw_busy_wait(microsec_byte); // TODO remove?
}


#endif //FRAMEWORK_LOG_ENABLED
