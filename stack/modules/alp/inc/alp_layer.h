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

/*! \file alp_layer.h
 * \addtogroup ALP
 * \ingroup D7AP
 * @{
 * \brief Application Layer Protocol APIs
 * \author	glenn.ergeerts@uantwerpen.be
 * \author	philippe.nunes@cortus.com
 * \author maarten.weyn@uantwerpen.be
 */

#ifndef ALP_LAYER_H_
#define ALP_LAYER_H_

#include "stdint.h"
#include "stdbool.h"

#include "fifo.h"
#include "alp.h"

#ifdef MODULE_D7AP
#include "d7ap.h"
#endif

typedef void (*alp_command_completed_callback)(uint8_t tag_id, bool success);
typedef void (*alp_command_result_callback)(alp_command_t* command, alp_interface_status_t* origin_itf_status);
typedef alp_status_codes_t (*alp_unhandled_read_action_callback)(const alp_interface_status_t* origin_itf_status, alp_operand_file_data_request_t operand, uint8_t* alp_response);

typedef struct {
    alp_command_completed_callback alp_command_completed_cb;
    alp_command_result_callback alp_command_result_cb;
    /**
     * @brief alp_unhandled_read_action_cb Called when the stack received an ALP read action which cannot be processed against the local filesystem,
     * because the requested fileID does not exist.
     * The application is given the chance to provide a response (by filling the alp_response parameter).
     * If the application is able to process the read action it should provide the data in alp_response and return ALP_STATUS_OK.
     * Otherwise, when it cannot handle the read action it should return ALP_STATUS_FILE_ID_NOT_EXISTS, or any other alp_status_codes_t item,
     * for other cases.
     * It is important to know this callback is called while a D7AP transaction is in process thus be sure to return within transaction timeout limits!
     */
    alp_unhandled_read_action_callback alp_unhandled_read_action_cb;
} alp_init_args_t;

typedef enum {
    ITF_START,
    ITF_STOP
} itf_ctrl_action_t;

typedef struct __attribute__((__packed__)){
    union {
        uint16_t raw_itf_ctrl;
        struct{
            itf_ctrl_action_t action;
            alp_itf_id_t interface;
        };
    };
} itf_ctrl_t;


/*!
 * \brief Initializes the ALP layer
 * \param init_args Specifies the callback function pointers
 * \param use_serial_interface Specifies if the ALP layer should initialize and use a serial interface
 */
void alp_layer_init(alp_init_args_t* init_args, bool forward_unsollicited_over_serial);

/*!
 * \brief Register a new interface in alp_layer
 * \param interface
 */
void alp_layer_register_interface(alp_interface_t* interface);

/*!
 * \brief Allocates an alp_command_t instance from the pool. Commands are always free-ed by ALP layer when completed.
 * \param with_tag_request Add a tag request action to the command
 * \param always_respond When set a response is requested even if there is no respond payload expected from other actions. Only relevant when `with_tag_request` is set
 * \return 
 */
alp_command_t* alp_layer_command_alloc(bool with_tag_request, bool always_respond);

/*!
 * \brief returns a pointer to the command with the given tag_id.
 * \param tag_id tag id from the command
 * \return pointer to command or NULL if command does not exist
 */
alp_command_t* alp_layer_get_command_by_tag_id(uint8_t tag_id);

/*!
 * \brief Free an allocated alp_command_t. Note that commands processed by ALP layer are always free-ed by ALP layer when completed already.
 * \param command Command to free
 * \return true if command got freed successfully
 */
bool alp_layer_command_free(alp_command_t* command);

/*!
 * \brief Free an alp_commands with the specified forward interface id. Note that commands processed by ALP layer are always free-ed by ALP layer when completed already.
 * \param forward_itf_id inteface id of commands that need to be free-ed
 * \return 
 */
void alp_layer_free_itf_commands(uint8_t forward_itf_id);

/*!
 * \brief Frees all commands in the ALP layer
 * \return 
 */
void alp_layer_free_commands();

/*!
 * \brief Processes the ALP command in an asynchronous way
 * \param Command Pointer to a command which should be allocated by calling `alp_layer_command_alloc()`. The command will be free-ed by ALP layer.
 * \return True if response payload is to be expected
 */
bool alp_layer_process(alp_command_t* command);

/*!
 * \brief To be called by an ALP interface implementation (only those which are aware of transactions) when a response is received.
 * \param trans_id The transaction id used to lookup the originating request
 * \param payload Response ALP payloader
 * \param payload_length Response length
 * \param itf_status The interface status
 */
void alp_layer_received_response(uint16_t trans_id, uint8_t* payload, uint8_t payload_length, alp_interface_status_t* itf_status); // TODO merge with alp_layer_process_command()?

/*!
 * \brief To be called by an ALP interface implementation (only those which are aware of transactions) when a command forwarded to this interface is completed.
 * \param trans_id The transaction id used to lookup the originating request
 * \param payload Response ALP payloader
 * \param payload_length Response length
 * \param itf_status The interface status
 * \param command_completed Is command fully completed 
 */
void alp_layer_forwarded_command_completed(uint16_t trans_id, error_t* error, alp_interface_status_t* status, bool command_completed);


#endif /* ALP_LAYER_H_ */

/** @}*/
