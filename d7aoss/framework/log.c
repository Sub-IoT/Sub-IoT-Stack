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
 */

#include "log.h"
#include "string.h"

#include "../hal/uart.h"

#include <stdio.h>
#include <stdarg.h>

//#define USE_SIMPLE_TERMINAL

// TODO only in debug mode?
#define BUFFER_SIZE 100
static char buffer[BUFFER_SIZE];

#pragma NO_HOOKS(log_print_string)
void log_print_string(char* format, ...)
{
    va_list args;
    va_start(args, format);
    uint8_t len = vsnprintf(buffer, BUFFER_SIZE, format, args);
    va_end(args);
#ifndef USE_SIMPLE_TERMINAL
    uart_transmit_data(0xDD);
    uart_transmit_data(LOG_TYPE_STRING);
    uart_transmit_data(len);
    uart_transmit_message((unsigned char*) buffer, len);
#else
    char buf[BUFFER_SIZE];
    sprintf(buf, "\n\r[%03d] %s", counter, buffer);
    uart_transmit_message((unsigned char*) buf, len+8);
    counter++;
#endif
}

#pragma NO_HOOKS(log_print_stack_string)
void log_print_stack_string(char type, char* format, ...)
{
    va_list args;
    va_start(args, format);
    uint8_t len = vsnprintf(buffer, BUFFER_SIZE, format, args);
    va_end(args);

#ifndef USE_SIMPLE_TERMINAL
    uart_transmit_data(0xDD);
    uart_transmit_data(LOG_TYPE_STACK);
    uart_transmit_data(type);
    uart_transmit_data(len);
    uart_transmit_message((unsigned char*) buffer, len);
#else
    char buf[BUFFER_SIZE];
    sprintf(buf, "\n\r[%03d] %s", counter, buffer);
    uart_transmit_message((unsigned char*) buf, len+8);
    counter++;
#endif
}

#pragma NO_HOOKS(log_print_trace)
void log_print_trace(char* format, ...)
{
    va_list args;
    va_start(args, format);
    uint8_t len = vsnprintf(buffer, BUFFER_SIZE, format, args);
    va_end(args);

#ifndef USE_SIMPLE_TERMINAL
    uart_transmit_data(0xDD);
    uart_transmit_data(LOG_TYPE_FUNC_TRACE);
    uart_transmit_data(len);
    uart_transmit_message((unsigned char*) buffer, len);
#else
    char buf[BUFFER_SIZE];
    sprintf(buf, "\n\r[%03d] %s", counter, buffer);
    uart_transmit_message((unsigned char*) buf, len+8);
    counter++;
#endif
}

void log_print_data(uint8_t* message, uint8_t length)
{
#ifndef USE_SIMPLE_TERMINAL
    uart_transmit_data(0xDD);
    uart_transmit_data(LOG_TYPE_DATA);
    uart_transmit_data(length);
    uart_transmit_message((unsigned char*) message, length);
#else
    char buf[BUFFER_SIZE], i;
    sprintf(buf, "\n\r[%03d]", counter);
    uart_transmit_message((unsigned char*) buf, 7);
    for( i=0 ; i<length ; i++ )
    {
        sprintf(buf, " %02X", message[i]);
        uart_transmit_message((unsigned char*) buf, 3);
    }
    counter++;
#endif
}

void log_phy_rx_res(phy_rx_data_t* res)
{
	// TODO: add channel id and frame_type
	// transmit the log header
	uart_transmit_data(0xDD);
	uart_transmit_data(LOG_TYPE_PHY_RX_RES);
	uart_transmit_data(LOG_TYPE_PHY_RX_RES_SIZE + res->length);

	// transmit struct member per member, so we are not dependent on packing
	uart_transmit_data(res->rssi);
	uart_transmit_data(res->lqi);
	uart_transmit_data(res->spectrum_id[1]);
	uart_transmit_data(res->spectrum_id[0]);
	uart_transmit_data(res->sync_word_class);
	uart_transmit_data(res->length);

	// transmit the packet
	uart_transmit_message(res->data, res->length);
}

void log_dll_rx_res(dll_rx_res_t* res)
{
	uart_transmit_data(0xDD);
	uart_transmit_data(LOG_TYPE_DLL_RX_RES);
	uart_transmit_data(LOG_TYPE_DLL_RX_RES_SIZE);
	uart_transmit_data(res->frame_type);
	uart_transmit_data(res->spectrum_id[1]);
	uart_transmit_data(res->spectrum_id[0]);
}

#ifdef LOG_TRACE_ENABLED
void __entry_hook(const char* function_name)
{
	log_print_trace("> %s", function_name);
}

void __exit_hook(const char* function_name)
{
	log_print_trace("< %s", function_name);
}
#endif
