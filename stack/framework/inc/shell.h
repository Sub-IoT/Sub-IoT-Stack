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

/*!
 * \file shell.h
 * \addtogroup shell
 * \ingroup framework
 * @{
 * \brief Implements a shell interface
 * \author glenn.ergeerts@uantwerpen.be
 */
#ifndef SHELL_H_
#define SHELL_H_

#include "framework_defs.h"

#define SHELL_CMD_HEADER_SIZE 4

#include "link_c.h"
#include "types.h"
#include "errors.h"
#include "fifo.h"

typedef void (*cmd_handler_t)(fifo_t* cmd_fifo);
typedef struct {
    int8_t id;
    cmd_handler_t cmd_handler_callback;
} cmd_handler_registration_t;

#ifdef FRAMEWORK_SHELL_ENABLED

void shell_init();
void shell_echo_enable();
void shell_echo_disable();
void shell_register_handler(cmd_handler_registration_t handler_registration);
void shell_return_output(uint8_t origin, uint8_t* data, uint8_t length);

#else

#define shell_init()                  ((void)0)
#define shell_echo_enable()           ((void)0)
#define shell_echo_disable()          ((void)0)
#define shell_register_handler(...)   ((void)0)
#define shell_return_output(...)      ((void)0)

#endif

#endif /* SHELL_H_ */

/** @}*/
