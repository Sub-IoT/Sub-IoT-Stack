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
 *     	glenn.ergeerts@uantwerpen.be
 *
 */

#ifndef __LOG_H_
#define __LOG_H_


// Set logging options in d7aoss.h
/*
 * TODO this is manually set in CSS!
 * to enable function tracing go to the d7aoss project properties:
 * build -> msp430 compiler -> Advanced options -> entry/exit hooks
 * Leave both fields empty and set the param fields to "name"
 * When LOG_TRACE_ENABLED is defined the functions will be listed.
 */

//#define LOG_TRACE_ENABLED

#include "../phy/phy.h"
#include "../dll/dll.h"
#include "../d7aoss.h"
#include "../hal/uart.h"

// generic logging functions
#define LOG_TYPE_STRING 0x01
#define LOG_TYPE_DATA 0x02
#define LOG_TYPE_STACK 0x03

// stack logging opcodes
#define LOG_PHY 0x01
#define LOG_DLL 0x02
#define LOG_MAC 0x03
#define LOG_NWL 0x04
#define LOG_TRANS 0x05
#define LOG_FWK 0x10
//TODO define others

// These are optional special logging types, use a higher value
#define LOG_TYPE_PHY_RX_RES 0xFE
#define LOG_TYPE_PHY_RX_RES_SIZE 8

#define LOG_TYPE_DLL_RX_RES 0xFD
#define LOG_TYPE_DLL_RX_RES_SIZE 2

#define LOG_TYPE_FUNC_TRACE 0xFF

// TODO: can be removed now log_print_string accepts a printf style format string?
//bool itoa(int32_t n, char* out);

// generic logging functions
void log_print_string(char* format, ...);
void log_print_stack_string(char type, char* format, ...);
void log_print_data(uint8_t* message, uint8_t length);

// special logging functions
void log_print_trace(char* format, ...);
void log_phy_rx_res(phy_rx_data_t* res);
void log_dll_rx_res(dll_rx_res_t* res);

#endif /* __LOG_H_ */
