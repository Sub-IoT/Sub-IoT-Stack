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
  STATE_NOT_INITIALIZED,
  STATE_INITIALIZED
} interface_state_t; //ADD MORE STATES?

typedef void (*alp_command_completed_callback)(uint8_t tag_id, bool success);
typedef void (*alp_command_result_callback)(alp_interface_status_t* result, uint8_t* payload, uint8_t payload_length);
typedef void (*alp_received_unsolicited_data_callback)(alp_interface_status_t* result, uint8_t *alp_command, uint8_t alp_command_size);
typedef alp_status_codes_t (*alp_unhandled_read_action_callback)(alp_interface_status_t* result, alp_operand_file_data_request_t operand, uint8_t* alp_response);

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
 * \param use_serial_interface Specifies if the ALP layer should initialize and use a serial interface
 */
void alp_layer_init(alp_init_args_t* init_args, bool use_serial_interface);


/*!
 * \brief Execute the command asynchronously against the provided session configuration
 * \param alp_command
 * \param alp_command_length
 * \param session_config
 */
void alp_layer_execute_command_over_itf(uint8_t* alp_command, uint8_t alp_command_length,  alp_interface_config_t* itf_cfg);

/*!
 * \brief Register a new interface in alp_layer
 * \param interface
 */
void alp_layer_register_interface(alp_interface_t* interface);

/*!
 * \brief Processes the ALP command in an asynchronous way
 * \param payload
 * \param len
 * \return True if response payload is to be expected
 */
bool alp_layer_process(alp_command_t* command);
void alp_layer_received_response(uint16_t trans_id, uint8_t* payload, uint8_t payload_length, alp_interface_status_t* itf_status); // TODO merge with alp_layer_process_command()?
void alp_layer_forwarded_command_completed(uint16_t trans_id, error_t* error, alp_interface_status_t* itf_status);
void alp_layer_process_d7aactp(d7ap_session_config_t* session_config, uint8_t* alp_command, uint32_t alp_command_length);


alp_command_t* alp_layer_command_alloc(bool with_tag_request, bool always_respond); // TODO expose?
alp_command_t* alp_layer_get_command_by_transid(uint16_t trans_id, uint8_t itf_id); // TODO alp_layer_forwarded_command_completed ?

#endif /* ALP_LAYER_H_ */

/** @}*/
