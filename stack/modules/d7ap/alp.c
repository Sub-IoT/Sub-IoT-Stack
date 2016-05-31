/*! \file alp.c
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
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include "debug.h"
#include "ng.h"

#include "alp.h"
#include "packet.h"
#include "fs.h"
#include "fifo.h"

alp_operation_t alp_get_operation(uint8_t* alp_command)
{
    alp_control_t alp_ctrl;
    alp_ctrl.raw = (*alp_command);
    return alp_ctrl.operation;
}

static uint8_t process_action(uint8_t* alp_action, uint8_t* alp_response, uint8_t* alp_response_length)
{

}

static uint8_t process_op_read_file_data(fifo_t* alp_command_fifo, fifo_t* alp_response_fifo) {
  alp_operand_file_data_request_t operand;
  error_t err;
  err = fifo_pop(alp_command_fifo, &operand.file_offset.file_id, 1); assert(err == SUCCESS);
  err = fifo_pop(alp_command_fifo, &operand.file_offset.offset, 1); assert(err == SUCCESS); // TODO can be 1-4 bytes, assume 1 for now
  err = fifo_pop(alp_command_fifo, &operand.requested_data_length, 1); assert(err == SUCCESS);

  // fill response
  fifo_put_byte(alp_response_fifo, ALP_OP_RETURN_FILE_DATA);
  fifo_put_byte(alp_response_fifo, operand.file_offset.file_id);
  fifo_put_byte(alp_response_fifo, operand.file_offset.offset); // TODO can be 1-4 bytes, assume 1 for now
  fifo_put_byte(alp_response_fifo, operand.requested_data_length);
  uint8_t data[operand.requested_data_length];
  alp_status_codes_t alp_status = fs_read_file(operand.file_offset.file_id, operand.file_offset.offset, data, operand.requested_data_length); // TODO status
  fifo_put(alp_response_fifo, data, operand.requested_data_length);
}

static uint8_t process_op_write_file_data(fifo_t* alp_command_fifo, fifo_t* alp_response_fifo) {
  alp_operand_file_data_t operand;
  error_t err;
  err = fifo_pop(alp_command_fifo, &operand.file_offset.file_id, 1); assert(err == SUCCESS);
  err = fifo_pop(alp_command_fifo, &operand.file_offset.offset, 1); assert(err == SUCCESS); // TODO can be 1-4 bytes, assume 1 for now
  err = fifo_pop(alp_command_fifo, &operand.provided_data_length, 1); assert(err == SUCCESS);
  uint8_t data[operand.provided_data_length];
  err = fifo_pop(alp_command_fifo, data, operand.provided_data_length);
  alp_status_codes_t alp_status = fs_write_file(operand.file_offset.file_id, operand.file_offset.offset, data, operand.provided_data_length); // TODO status
}

static uint8_t process_op_forward(fifo_t* alp_command_fifo, fifo_t* alp_response_fifo, d7asp_fifo_config_t* fifo_config) {
  uint8_t interface_id;
  error_t err;
  err = fifo_pop(alp_command_fifo, &interface_id, 1); assert(err == SUCCESS);
  assert(interface_id == ALP_ITF_ID_D7ASP); // only D7ASP supported for now // TODO return error instead of asserting
  err = fifo_pop(alp_command_fifo, &fifo_config->qos.raw, 1); assert(err == SUCCESS);
  err = fifo_pop(alp_command_fifo, &fifo_config->dormant_timeout, 1); assert(err == SUCCESS);
  err = fifo_pop(alp_command_fifo, &fifo_config->addressee.ctrl.raw, 1); assert(err == SUCCESS);
  uint8_t id_length = d7anp_addressee_id_length(fifo_config->addressee.ctrl.id_type);
  err = fifo_pop(alp_command_fifo, fifo_config->addressee.id, id_length); assert(err == SUCCESS);
}

bool alp_process_command(uint8_t* alp_command, uint8_t alp_command_length, uint8_t* alp_response, uint8_t* alp_response_length)
{
  (*alp_response_length) = 0;
  d7asp_fifo_config_t d7asp_fifo_config;
  bool do_forward = false;

  fifo_t alp_command_fifo, alp_response_fifo;
  fifo_init_filled(&alp_command_fifo, alp_command, alp_command_length, alp_command_length);
  fifo_init(&alp_response_fifo, alp_response, ALP_PAYLOAD_MAX_SIZE);

  while(fifo_get_size(&alp_command_fifo) > 0) {
    if(do_forward) {
      // forward rest of the actions over the D7ASP interface
      uint8_t forwarded_alp_size = fifo_get_size(&alp_command_fifo);
      uint8_t forwarded_alp_actions[forwarded_alp_size];
      fifo_pop(&alp_command_fifo, forwarded_alp_actions, forwarded_alp_size);
      d7asp_queue_alp_actions(&d7asp_fifo_config, forwarded_alp_actions, forwarded_alp_size); // TODO pass fifo directly?
      break; // TODO return response
    }

    alp_control_t control;
    fifo_pop(&alp_command_fifo, &control.raw, 1);
    switch(control.operation) {
      case ALP_OP_READ_FILE_DATA:
        process_op_read_file_data(&alp_command_fifo, &alp_response_fifo);
        break;
      case ALP_OP_WRITE_FILE_DATA:
        process_op_write_file_data(&alp_command_fifo, &alp_response_fifo);
        break;
      case ALP_OP_FORWARD:
        process_op_forward(&alp_command_fifo, &alp_response_fifo, &d7asp_fifo_config);
        do_forward = true;
        break;
      default:
        assert(false); // TODO return error
        //alp_status = ALP_STATUS_UNKNOWN_OPERATION;
    }
  }

  (*alp_response_length) = fifo_get_size(&alp_response_fifo);

    // TODO return ALP status if requested

//    if(alp_status != ALP_STATUS_OK)
//      return false;

    return true;
}

