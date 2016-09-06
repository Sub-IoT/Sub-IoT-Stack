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

/*
 * \author	glenn.ergeerts@uantwerpen.be
 */

#ifndef ALP_CMD_HANDLER_H
#define ALP_CMD_HANDLER_H

#include "types.h"
#include "fifo.h"
#include "d7asp.h"


#define ALP_CMD_HANDLER_ID 'D'

typedef void (*alp_cmd_handler_appl_itf_callback)(uint8_t* alp_command, uint8_t alp_command_length);

///
/// \brief Shell command handler for ALP interface
/// \param cmd_fifo
///
void alp_cmd_handler(fifo_t* cmd_fifo);

///
/// \brief Output ALP command to the shell interface
/// \param alp_command
/// \param alp_command_len
///
void alp_cmd_handler_output_alp_command(uint8_t *alp_command, uint8_t alp_command_len);

///
/// \brief Output received responses received from D7ASP to the shell interface
/// \param d7asp_result
/// \param alp_command
/// \param alp_command_size
///
void alp_cmd_handler_output_d7asp_response(d7asp_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size);

///
/// \brief Output the completion of a Command to the shell interface
/// \param tag_id The tag ID of the completed command.
/// \param error When set an error occured during processing of the command
///
void alp_cmd_handler_output_command_completed(uint8_t tag_id, bool error);

///
/// \brief Set the callback which will be called when an ALP command is received for the application interface
/// \param cb
///
void alp_cmd_handler_set_appl_itf_callback(alp_cmd_handler_appl_itf_callback cb);

#endif // ALP_CMD_HANDLER_H
