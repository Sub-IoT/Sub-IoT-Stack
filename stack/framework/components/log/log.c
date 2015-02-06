/*! \file log.c
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

#ifdef LOGGING_ENABLED


// generic logging functions
#define LOG_TYPE_STRING 0x01
#define LOG_TYPE_DATA 0x02
#define LOG_TYPE_STACK 0x03




#ifdef LOG_BINARY
	#define BUFFER_SIZE 100
	static char NGDEF(buffer)[BUFFER_SIZE];
#else
	static uint32_t NGDEF(counter);
#endif //LOG_BINARY

void log_counter_reset()
{
#ifndef LOG_BINARY
	NG(counter) = 0;
#endif //LOG_BINARY
}

void log_print_string(char* format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef LOG_BINARY
    uint8_t len = vsnprintf(NG(buffer), BUFFER_SIZE, format, args);
    putc(0xDD,stdout);
    putc(LOG_TYPE_STRING,stdout);
    putc(len,stdout);
    fwrite(NG(buffer),len, sizeof(char), stdout);
    fflush(stdout);
#else
    printf("\n\r[%03d] ", NG(counter)++);
    vprintf(format, args);
#endif //LOG_BINARY
    va_end(args);

}

void log_print_stack_string(char type, char* format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef LOG_BINARY
    uint8_t len = vsnprintf(NG(buffer), BUFFER_SIZE, format, args);
    putc(0xDD,stdout);
    putc(LOG_TYPE_STACK,stdout);
    putc(type,stdout);
    putc(len,stdout);
    fwrite(NG(buffer),len, sizeof(char), stdout);
    fflush(stdout);
#else
    printf("\n\r[%03d] ", NG(counter)++);
    vprintf(format, args);
#endif //LOG_BINARY
    va_end(args);
}

void log_print_data(uint8_t* message, uint8_t length)
{
#ifdef LOG_BINARY
    putc(0xDD,stdout);
    putc(LOG_TYPE_DATA,stdout);
    putc(length,stdout);
    fwrite(message, length, sizeof(uint8_t), stdout);
    fflush(stdout);
#else
    printf("\n\r[%03d]", NG(counter)++);
    for( uint8_t i=0 ; i<length ; i++ )
    {
    	printf(" %02X", message[i]);
    }
#endif //LOG_BINARY
}


#endif //LOGGING_ENABLED
