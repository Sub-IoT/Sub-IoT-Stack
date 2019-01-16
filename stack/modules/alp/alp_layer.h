/*! \file alp_layer.h
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
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
 *
 * \author glenn.ergeerts@uantwerpen.be
 * \author maarten.weyn@uantwerpen.be
 *
 */

/*! \file alp_layer.h
 * \addtogroup ALP
 * \ingroup D7AP
 * @{
 * \brief Application Layer Protocol APIs
 * \author	glenn.ergeerts@uantwerpen.be
 * \author	philippe.nunes@cortus.com
 */

#ifndef ALP_LAYER_H_
#define ALP_LAYER_H_

#include "stdint.h"
#include "stdbool.h"

#include "fifo.h"
#include "alp.h"
#include "d7ap.h"
#include "lorawan_stack.h"

typedef enum
{
    ALP_CMD_ORIGIN_APP,
    ALP_CMD_ORIGIN_SERIAL_CONSOLE,
    ALP_CMD_ORIGIN_D7AACTP,
    ALP_CMD_ORIGIN_D7AP,
} alp_command_origin_t;

typedef enum
{
  STATE_NOT_INITIALIZED,
  STATE_INITIALIZED
} interface_state_t; //ADD MORE STATES?



typedef void (*alp_command_completed_callback)(uint8_t tag_id, bool success);
typedef void (*alp_command_result_callback)(d7ap_session_result_t result, uint8_t* payload, uint8_t payload_length);
typedef void (*alp_received_unsolicited_data_callback)(d7ap_session_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size);
typedef alp_status_codes_t (*alp_unhandled_read_action_callback)(d7ap_session_result_t d7asp_result, alp_operand_file_data_request_t operand, uint8_t* alp_response);

typedef struct {
    alp_command_completed_callback alp_command_completed_cb;
    alp_command_result_callback alp_command_result_cb;
    alp_received_unsolicited_data_callback alp_received_unsolicited_data_cb;
    /**
     * @brief alp_unhandled_read_action_cb Called when the stack received an ALP read action which cannot be processed against the local filesystem,
     * because the requested fileID does not exist.
     * The application is given the chance to provide a response (by filling the alp_response and alp_response_length parameters).
     * If the application is able to process the read action it should provide the data in alp_response and return ALP_STATUS_OK.
     * Otherwise, when it cannot handle the read action it should return ALP_STATUS_FILE_ID_NOT_EXISTS, or any other alp_status_codes_t item,
     * for other cases.
     * It is important to know this callback is called while a D7AP transaction is in process thus be sure to return within transaction timeout limits!
     */
    alp_unhandled_read_action_callback alp_unhandled_read_action_cb;
} alp_init_args_t;



/*!
 * \brief Initializes the ALP layer
 * \param init_args Specifies the callback function pointers
 * \param shell_enabled Specifies if ALP is accessible over the serial console
 */
void alp_layer_init(alp_init_args_t* init_args, bool shell_enabled);


/*!
 * \brief Execute the command asynchronously against the provided D7ASP session configuration
 * \param alp_command
 * \param alp_command_length
 * \param d7asp_master_session_config
 */
void alp_layer_execute_command_over_d7a(uint8_t* alp_command, uint8_t alp_command_length, d7ap_session_config_t* config);

/*!
 * \brief Process the ALP command.
 * Processing will be done against the local host interface unless explicitely forwarded to another interface using an (indirect) forward action.
 *
 * Note: alp_command and alp_response may point to the same buffer
 * \param alp_command   The raw command
 * \param alp_command_length The length of the command
 * \param alp_response Pointer to a buffer where a possible response will be written
 * \param alp_response_length The length of the response
 * \param origin Where the ALP command originates from, determines where response will go to
 * \return If the ALP command was processed correctly or not
 */
bool alp_layer_process_command(uint8_t* alp_command, uint8_t alp_command_length, uint8_t* alp_response, uint8_t* alp_response_length, alp_command_origin_t origin);

/*!
 * \brief Process a result received from D7ASP.
 *
 * Note: alp_command and alp_response may point to the same buffer
 * \param alp_command         The raw command
 * \param alp_command_length  The length of the command
 * \param alp_response        Pointer to a buffer where a possible response will be written
 * \param alp_response_length The length of the response
 * \param d7asp_result        The D7AP session result
 */
void alp_layer_process_d7ap_result(uint8_t* alp_command, uint8_t alp_command_length, d7ap_session_result_t d7asp_result);

/*!
 * \brief Process the ALP command on the local host interface and output the response to the D7ASP interface
 *
 * \param d7asp_fifo_config The config of the D7ASP fifo to output the ALP response to
 * \param alp_command   The raw command
 * \param alp_command_length The length of the command
 */
void alp_layer_process_d7aactp(d7ap_session_config_t* config, uint8_t* alp_command, uint32_t alp_command_length);

/*!
 * \brief Process the ALP command and output the result on the console.
 * Processing will be done against the local host interface unless explicitely forwarded to another interface using an (indirect) forward action.
 *
 * Note: alp_command and alp_response may point to the same buffer
 * \param alp_command   The raw command
 * \param alp_command_length The length of the command
 */
void alp_layer_process_command_console_output(uint8_t* alp_command, uint8_t alp_command_length);

void alp_layer_d7ap_session_completed(void);

#endif /* ALP_LAYER_H_ */

/** @}*/
