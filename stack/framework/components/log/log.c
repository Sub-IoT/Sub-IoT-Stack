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
#include "framework_defs.h"

#ifdef FRAMEWORK_LOG_ENABLED


// generic logging functions
#define LOG_TYPE_STRING 0x01
#define LOG_TYPE_DATA 0x02
#define LOG_TYPE_STACK 0x03


#ifdef FRAMEWORK_LOG_BINARY
	#define BUFFER_SIZE 100
	static char NGDEF(buffer)[BUFFER_SIZE];
#else
	static uint32_t NGDEF(counter);
#endif //FRAMEWORK_LOG_BINARY

__LINK_C void log_counter_reset()
{
#ifndef FRAMEWORK_LOG_BINARY
	NG(counter) = 0;
#endif //FRAMEWORK_LOG_BINARY

}

__LINK_C void log_print_string(char* format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef FRAMEWORK_LOG_BINARY
    uint8_t len = vsnprintf(NG(buffer), BUFFER_SIZE, format, args);
    putc(0xDD,stdout);
    putc(LOG_TYPE_STRING,stdout);
    putc(len,stdout);
    fwrite(NG(buffer),len, sizeof(char), stdout);
#else
    printf("\n\r[%03d] ", NG(counter)++);
    vprintf(format, args);
#endif //FRAMEWORK_LOG_BINARY
    va_end(args);
    fflush(stdout);
}

__LINK_C void log_print_stack_string(log_stack_layer_t type, char* format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef FRAMEWORK_LOG_BINARY
    uint8_t len = vsnprintf(NG(buffer), BUFFER_SIZE, format, args);
    putc(0xDD,stdout);
    putc(LOG_TYPE_STACK,stdout);
    putc(type,stdout);
    putc(len,stdout);
    fwrite(NG(buffer),len, sizeof(char), stdout);
#else
    printf("\n\r[%03d] ", NG(counter)++);
    vprintf(format, args);
#endif //FRAMEWORK_LOG_BINARY
    va_end(args);
    fflush(stdout);
}

__LINK_C void log_print_data(uint8_t* message, uint32_t length)
{
#ifdef FRAMEWORK_LOG_BINARY
    putc(0xDD,stdout);
    putc(LOG_TYPE_DATA,stdout);
    putc(length,stdout);
    fwrite(message, length, sizeof(uint8_t), stdout);
#else
    printf("\n\r[%03d]", NG(counter)++);
    for( uint32_t i=0 ; i<length ; i++ )
    {
    	printf(" %02X", message[i]);
    }
#endif //FRAMEWORK_LOG_BINARY
    fflush(stdout);
}


#endif //FRAMEWORK_LOG_ENABLED
