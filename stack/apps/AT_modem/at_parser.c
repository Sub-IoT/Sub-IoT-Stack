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

/*
 *  Authors:
 *  philippe.nunes@cortus.com
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "framework_defs.h"
#include "at_parser.h"

#ifdef FRAMEWORK_LOG_ENABLED
#include "log.h"
        #define DPRINT(...) log_print_string(__VA_ARGS__)
        #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
    #define DPRINT(...)
    #define DPRINT_DATA(...)
#endif

extern AT_COMMAND cmd_items[AT_COMMANDS_NUM];

void ms_array_slice_to_string(string_t array, uint16_t start, uint16_t end, char *ret)
{
    uint16_t i, n = 0;

    for(i = start; i <= end; i++)
    {
        ret[n] = array[i];
        n++;
    }

    ret[n] = 0;
}

void at_register_command(string_t command, at_callback getter, at_callback setter /*, at_callback test, at_callback execute*/)
{
    int i;

    for(i = 0; i < AT_COMMANDS_NUM; i++)
    {
        if(cmd_items[i].cmd == NULL)
        {
            char *new_cmd = malloc(strlen(command) + 1);
            strncpy(new_cmd, command, strlen(command));
            cmd_items[i].cmd = new_cmd;
            cmd_items[i].getter = getter;
            cmd_items[i].setter = setter;
            return;
        }
    }
}

char at_execute_command(string_t command, char *value, unsigned char type)
{
    int i;
    char result = AT_ERROR;

    for(i = 0; cmd_items[i].cmd != NULL; i++)
    {
        if(strcmp((const char *)command, cmd_items[i].cmd) == 0)
        {            
            switch(type)
            {
                case AT_PARSER_STATE_WRITE:
                    if(cmd_items[i].setter)
                        result = cmd_items[i].setter(value);
                    break;
                case AT_PARSER_STATE_READ:
                    if(cmd_items[i].getter)
                        result = cmd_items[i].getter(value);
                    break;
                case AT_PARSER_STATE_TEST:
                    //if(cmd_items[i].test)
                    //    result = at_registered_commands[i].test(value);
                    break;
                case AT_PARSER_STATE_COMMAND:
                    //if(at_registered_commands[i].execute)
                    //    result = at_registered_commands[i].execute(value);
                    if(cmd_items[i].setter) // setter without any argument is considered as a command
                        result = cmd_items[i].setter(value);
                    break;
                default:
                    result = AT_ERROR;
            }

            goto send_result;
        }
    }

send_result:
    return result;
}

/*
 
 AT+COMMAND=? -> List
 AT+COMMAND=VALUE -> Write
 AT+COMMAND? -> Read
 AT+COMMAND -> Execute
 
 */

char at_parse_line(string_t line, char *ret)
{
    uint16_t i;
    char result = AT_ERROR;
    char state = AT_PARSER_STATE_COMMAND;

    char *at_cmd = strstr((const char *)line, (const char *)AT_COMMAND_MARKER);
    uint16_t line_len = strlen((const char *)line);
    int16_t index_write_start = -1;
    int16_t index_command_end = line_len - 1;
    char temp[AT_MAX_TEMP_STRING];

    if (at_cmd == NULL)
        return AT_ERROR;

    // Skip the marker
    int16_t start = (at_cmd - (char*)line) +  strlen((const char *)AT_COMMAND_MARKER);

    for(i = start; i < line_len; i++)
    {
        // Execute 'read' command
        if(line[i] == '?' && state == AT_PARSER_STATE_COMMAND)
        {
            index_command_end = i - 1;
            state = AT_PARSER_STATE_READ;
        }
        else if(line[i] == '=' && state == AT_PARSER_STATE_COMMAND)
        {
            index_command_end = i - 1;

            if(i < (line_len - 1))
            {
                if(line[i + 1] == '?')
                {
                    state = AT_PARSER_STATE_TEST;
                }
                else
                {
                    index_write_start = i + 1;
                    state = AT_PARSER_STATE_WRITE;
                }
            }
            else
            {
                return AT_ERROR;
            }
        }
    }

    ret[0] = 0;

    switch(state)
    {
        case AT_PARSER_STATE_COMMAND:
        case AT_PARSER_STATE_READ:
        case AT_PARSER_STATE_TEST:
            ms_array_slice_to_string(line, start, index_command_end, temp);
            result = at_execute_command(temp, ret, state);
            break;

        case AT_PARSER_STATE_WRITE:
            ms_array_slice_to_string(line, start, index_command_end, temp);
            if(index_write_start <= (line_len - 1))
            {
                ms_array_slice_to_string(line, index_write_start, line_len - 1, ret);
                result = at_execute_command(temp, ret, state);
                ret[0] = 0;
            }
            else
            {
                result = at_execute_command(temp, ret, state);
                ret[0] = 0;
            }
            break;

        default:
            return AT_ERROR;
    }

    return result;
}


char at_parse_extract_number(string_t parameter, uint32_t *number)
{
    int pos = 0;
    uint32_t value = 0;

    while (parameter[pos] >= '0' && parameter[pos] <= '9') {
        value = value * 10 + (uint32_t)(parameter[pos] - '0');
        pos += 1;
    }

    if (pos == 0)
        return AT_ERROR;

    if (number)
        *number = value;

    return AT_OK;
}

char at_parse_extract_hexstring(string_t parameter, uint8_t *bytes, uint8_t *bytes_len)
{
    int pos = 0;
    int len = strlen((const char *)parameter);

    // check if the size of the string is compatible with the size of the expected number
    if ((len%2) || (*bytes_len < (len / 2)))
        return AT_ERROR;

    while (pos < len)
    {
        if ((parameter[pos] >= '0' && parameter[pos] <= '9') ||
           (parameter[pos] >= 'a' && parameter[pos] <= 'f') ||
           (parameter[pos] >= 'A' && parameter[pos] <= 'F'))
            pos += 1;
        else
            return AT_ERROR;
    }

    *bytes_len = pos / 2;
    pos = 0;

    for (int i = 0; i < *bytes_len; i++)
    {
        unsigned byte ;
        sscanf( &parameter[i * 2], "%02X", &byte ) ;

        DPRINT("byte <%x>", byte);

        //DPRINT("parameter + pos %s ", &parameter[pos]);
        //sscanf(&parameter[pos], "%02hhx", buf);

        bytes[i] = byte;
        DPRINT("byte[%d]=0x%X", i, bytes[i]);
    }

    return AT_OK;
}

