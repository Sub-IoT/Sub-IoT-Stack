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

// *************************************************************************************************
// Radio core access functions. Taken from TI reference code for CC430.
// *************************************************************************************************

#include <stdbool.h>
#include <stdint.h>


#include "si4455_interface.h"
#include "log.h"
#include "debug.h"

// turn on/off the debug prints
#ifdef FRAMEWORK_LOG_ENABLED // TODO more granular
    #define DPRINT(...) log_print_stack_string(LOG_STACK_PHY, __VA_ARGS__)
#else
    #define DPRINT(...)
#endif

