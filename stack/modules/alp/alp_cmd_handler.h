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


/*! \file alp_cmd_handler.h
 * \addtogroup alp_cmd_handler
 * \ingroup D7AP
 * @{
 * \brief APIs to handle ALP through the shell interface
 * \author	glenn.ergeerts@uantwerpen.be
 */

#ifndef ALP_CMD_HANDLER_H
#define ALP_CMD_HANDLER_H

#include "types.h"
#include "fifo.h"
#include "d7ap.h"
#include "alp_layer.h"

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
void alp_cmd_handler_output_alp_command(fifo_t* resp_fifo);

///
/// \brief Output received responses received from D7ASP to the shell interface
/// \param d7asp_result
/// \param alp_command
/// \param alp_command_size
///
void alp_cmd_handler_output_response(alp_interface_status_t result, uint8_t *alp_command, uint8_t alp_command_size);

///
/// \brief Set the callback which will be called when an ALP command is received for the application interface
/// \param cb
///
void alp_cmd_handler_set_appl_itf_callback(alp_cmd_handler_appl_itf_callback cb);

///
/// \brief Modem interface command handler for ALP interface
/// \param cmd_fifo
///
void modem_interface_cmd_handler(fifo_t* cmd_fifo);

#endif // ALP_CMD_HANDLER_H

/** @}*/
