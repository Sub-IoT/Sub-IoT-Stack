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
#include "framework_defs.h"
#include "hwsystem.h"
#include "SEGGER_RTT.h"

#ifdef FRAMEWORK_LOG_ENABLED


static uint32_t NGDEF(counter);


__LINK_C void log_counter_reset()
{
	NG(counter) = 0;
}

__LINK_C void log_print_string(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    printf("\n\r[%03ld] ", NG(counter)++);
    vprintf(format, args);
    va_end(args);
}

__LINK_C void log_print_stack_string(log_stack_layer_t type, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    printf("\n\r[%03ld] ", NG(counter)++);
    vprintf(format, args);
    va_end(args);
}

__LINK_C void log_print_data(uint8_t* message, uint32_t length)
{
    printf("\n\r[%03ld]", NG(counter)++);
    for( uint32_t i=0 ; i<length ; i++ )
    {
        printf(" %02X", message[i]);
        //printf will enter an infinite loop when we enter to many characters without a newline character
        if(((i % 100) == 0) && i)
            printf("\n\r");
    }
}

void log_print_error_string(const char* format,...)
{
    va_list args;
    va_start(args, format);
    printf("\n\r%s[%03ld]%s ", RTT_CTRL_BG_BRIGHT_RED, NG(counter)++, RTT_CTRL_RESET);
    vprintf(format, args);
    va_end(args);
}


#endif //FRAMEWORK_LOG_ENABLED
