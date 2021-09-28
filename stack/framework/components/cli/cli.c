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
 * \file cli.c
 * \author contact@christophe.vg
 */

#include "framework_defs.h"
#ifdef FRAMEWORK_CONSOLE_ENABLED

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "fifo.h"

#include "console.h"
#include "shell.h"
#include "cli.h"



// TODO configure this elsewhere, in another way
#define CLI_CMDS 10

struct cli_command {
  char          component;
  char          command;
  char*         description;
  cli_handler_t handler;
};

static cli_command_t commands[CLI_CMDS];
static uint8_t       last_command = 0;

static char* cli_name;
static char  cli_id;

static uint8_t consume(fifo_t* cmd) {
  uint8_t buffer;
  fifo_pop(cmd, &buffer, 1);
  return buffer;
}

static uint8_t peek(fifo_t* input, uint8_t pos) {
  uint8_t buffer;
  fifo_peek(input, &buffer, pos, 1);
  return buffer;
}

static void drop(fifo_t* input) {
  while(consume(input) != '\r');
}

static void drop_invalid(fifo_t* input) {
  console_print("ERROR: invalid command\r\n");
  drop(input);
}

static void cli_handle_command(fifo_t* input) {
  if(fifo_get_size(input) < 7) { return; } // not enough AT$...\r

  // wait for a complete command, ending with \r
  bool complete = false;
  for(uint8_t i=3; ! complete && i<fifo_get_size(input); i++) {
    complete = peek(input, i) == '\r';
  }
  if( ! complete ) { return; }

  // consume AT$.
  consume(input); consume(input); consume(input); consume(input);

  char component = (char)consume(input);
  char command   = (char)consume(input);
  
  for(uint8_t c=0; c<last_command; c++) {
    if(commands[c].component == component && commands[c].command == command) {
      commands[c].handler(input);
      break;
    }
  }
  
  drop(input);
}

static void cli_info() {  
  console_print("\r\nThis is an interactive shell.\r\n");
  console_print("For example: try typing ATR followed by enter.\r\n");
  console_print("The node will reboot\r\n\r\n");

  console_printf("%s-specific commands are formatted as AT$%cxy\\r\r\n\r\n",
                 cli_name, cli_id);
  console_print("with x being a component and y being a command for that components.\r\n");
}

// public API

void cli_init(char* name, char id) {
  cli_name = name;
  cli_id   = id;

  shell_echo_enable();
  shell_register_handler((cmd_handler_registration_t) {
    .id                   = id,
    .cmd_handler_callback = &cli_handle_command
  });
  
  cli_info();
}

void cli_register_command(char component, char command, char* description,
                          cli_handler_t handler)
{
  assert(last_command < CLI_CMDS);
  commands[last_command] = (cli_command_t){
    .component   = component,
    .command     = command,
    .description = description,
    .handler     = handler
  };
  last_command++;
}

void cli_help() {
  console_print( "\r\nAvailable commands:\r\n");
  for(uint8_t c=0; c<last_command; c++) {
    console_printf(
      "  %c   %c   %s\r\n",
      commands[c].component, commands[c].command, commands[c].description
    );
  }
  console_print("\r\n");
}

#endif
