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


// functions used by cc1101_interface.c when accessing a CC1101 over SPI.
// when using CCS instead of cmake make sure to exclude this file from the build

#include "stdint.h"
#include "debug.h"

#include "hwspi.h"
#include "hwgpio.h"
#include "hwsystem.h"
#include "timer.h"

//#include "cc1101_constants.h"
#include "si4455_interface.h"

 #include "platform.h"


// turn on/off the debug prints
#ifdef LOG_PHY_ENABLED
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

static end_of_packet_isr_t end_of_packet_isr_callback;

