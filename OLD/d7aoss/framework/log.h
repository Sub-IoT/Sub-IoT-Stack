/*! \file log.h
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
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
void log_print(uint8_t* message);
void log_printf(char* format, ...);
void log_printf_stack(char type, char* format, ...);
void log_print_data(uint8_t* message, uint8_t length);

// special logging functions
void log_print_trace(char* format, ...);
void log_phy_rx_res(phy_rx_data_t* res);
void log_dll_rx_res(dll_rx_res_t* res);

#endif /* __LOG_H_ */
