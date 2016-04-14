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

/*! \file d7ap_stack.h
 * \defgroup D7AP
 * @{
 * \brief The D7AP stack implementation
 *
 * \author glenn.ergeerts@uantwerpen.be
 */

#ifndef OSS_7_D7AP_STACK_H
#define OSS_7_D7AP_STACK_H

#include "fs.h"
#include "alp.h"
#include "alp_cmd_handler.h"
#include "d7atp.h"
#include "d7asp.h"
#include "packet_queue.h"
#include "dll.h"

/**
 * @brief Initializes the D7AP stack, should be called by all applications making use of the D7AP stack
 * @param fs_init_args
 * @param d7asp_init_args
 * @param enable_shell
 * @param Called when the shell receives ALP commands for interface ID application.
 */
void d7ap_stack_init(fs_init_args_t* fs_init_args, d7asp_init_args_t* d7asp_init_args, bool enable_shell, alp_cmd_handler_appl_itf_callback cb);

#endif //OSS_7_D7AP_STACK_H

/** @}*/
