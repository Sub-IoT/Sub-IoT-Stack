/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV, CORTUS.
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

/*! \file at_parser.h
 * \author philippe.nunes@cortus.com
 */

#ifndef __AT_PARSER_H
#define __AT_PARSER_H

#include "types.h"

#define AT_MAX_TEMP_STRING 50
#define AT_COMMANDS_NUM    30

typedef char (*at_callback)(char *args_at);

typedef struct _at_command
{
    const char* cmd;
    at_callback setter;
    at_callback getter;
    //at_callback test;
    //at_callback execute;
    const char* const arg_descr;
    const char* const description;
} AT_COMMAND;

#define AT_OK       0
#define AT_ERROR    1

#define AT_PARSER_STATE_COMMAND     0
#define AT_PARSER_STATE_TEST        1
#define AT_PARSER_STATE_READ        2
#define AT_PARSER_STATE_WRITE       3
#define AT_PARSER_STATE_EQUAL       4

#ifndef AT_COMMAND_MARKER
#define AT_COMMAND_MARKER "AT"
#endif

void at_register_command(string_t command, at_callback getter, at_callback setter/*, at_callback test, at_callback execute*/);
char at_parse_line(string_t line, char *ret);
char at_parse_extract_number(string_t parameter, uint32_t *number);
char at_parse_extract_hexstring(string_t parameter, uint8_t *bytes, uint8_t *bytes_len);

#endif //__AT_PARSER_H
