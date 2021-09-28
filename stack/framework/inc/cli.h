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
 * \file cli.h
 * \addtogroup cli
 * \ingroup framework
 * @{
 * \brief Implements a shell-based cli interface
 * \author contact@christophe.vg
 */

#ifndef CLI_H_
#define CLI_H_

#include "fifo.h"

typedef void (*cli_handler_t)(fifo_t*);
typedef struct cli_command cli_command_t;

void cli_init(char* name, char id);
void cli_register_command(char component, char command, char* description,
                          cli_handler_t handler);
void cli_help(void);

#endif

/** @}*/
