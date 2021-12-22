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


// This examples pushes sensor data to gateway(s) by manually constructing an ALP command with a file read result action
// (unsolicited message). The D7 session is configured to request ACKs. All received ACKs are printed.
// Temperature data is used as a sensor value, when a HTS221 is available, otherwise value 0 is used.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hwleds.h"
#include "hwsystem.h"
#include "hwlcd.h"

#include "scheduler.h"
#include "timer.h"
#include "debug.h"
#include "d7ap_fs.h"
#include "log.h"

#include "d7ap.h"
#include "alp_layer.h"
#include "dae.h"

#include "platform.h"

#ifdef USE_HTS221
  #include "HTS221_Driver.h"
  #include "hwi2c.h"
#endif

#define SENSOR_FILE_ID           0x42
#define SENSOR_FILE_SIZE         2
#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC * 10

#define APPLICATION_FILE_ID      0x43

#ifdef USE_HTS221
  static i2c_handle_t* hts221_handle;
#endif

// Define the D7 interface configuration used for sending the ALP command on

#define QUERY_FILE_ID 0x43

static alp_interface_config_d7ap_t itf_config = (alp_interface_config_d7ap_t){
  .itf_id = ALP_ITF_ID_D7ASP,
  .d7ap_session_config = {
    .qos = {
        .qos_resp_mode = SESSION_RESP_MODE_ALL,
        .qos_retry_mode = SESSION_RETRY_MODE_NO
    },
    .dormant_timeout = 0,
    .addressee = {
        .ctrl = {
            .nls_method = AES_NONE,
            .id_type = ID_TYPE_NOID,
        },
        .access_class = 0x01,
        .id = { 0 }
    }
  }
};

static alp_operand_query_t query = (alp_operand_query_t) { 
    .code = { 
        .type = QUERY_CODE_TYPE_ARITHM_COMP_WITH_VALUE_IN_QUERY,
        .mask = false,
        .param = ARITH_COMP_TYPE_GREATER_THAN_OR_EQUAL_TO 
    },
    .compare_operand_length = 1
};

void execute_sensor_measurement()
{
  // first get the sensor reading ...
  int16_t temperature = 0; // in decicelsius. When there is no sensor, we just transmit 0 degrees

#if defined USE_HTS221
  i2c_acquire(hts221_handle);
  HTS221_Get_Temperature(hts221_handle, &temperature);
  i2c_release(hts221_handle);
#endif

  temperature = __builtin_bswap16(temperature); // convert to big endian before transmission

  // Generate ALP command.
  // We will be sending a return file data action, without a preceding file read request.
  // This is an unsolicited message, where we push the sensor data to the gateway(s).
  
  // alloc command. This will be freed when the command completes
  alp_command_t* command = alp_layer_command_alloc(false, false);
  
  // forward to the D7 interface
  alp_append_forward_action(command, (alp_interface_config_t*)&itf_config, sizeof(itf_config));

  alp_append_break_query_action(command, QUERY_FILE_ID, 0, &query);

  // add the return file data action
  alp_append_return_file_data_action(command, SENSOR_FILE_ID, 0, SENSOR_FILE_SIZE, (uint8_t*)&temperature);

  // and finally execute this
  alp_layer_process(command);
}

void on_alp_command_completed_cb(uint8_t tag_id, bool success)
{
    if(success)
      log_print_string("Command (%i) completed successfully", tag_id);
    else
      log_print_string("Command failed, no ack received");

    // reschedule sensor measurement
    timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);
}

void on_alp_command_result_cb(alp_command_t *alp_command, alp_interface_status_t* origin_itf_status)
{
  if(origin_itf_status && (origin_itf_status->itf_id == ALP_ITF_ID_D7ASP) && (origin_itf_status->len > 0)) {
      d7ap_session_result_t* d7_result = ((d7ap_session_result_t*)origin_itf_status->itf_status);
      log_print_string("recv response @ %i dB link budget from:", d7_result->rx_level);
      log_print_data(d7_result->addressee.id, d7ap_addressee_id_length(d7_result->addressee.ctrl.id_type));
  }
  log_print_string("response payload:");
  log_print_data(alp_command->alp_command, fifo_get_size(&alp_command->alp_command_fifo));
  fifo_skip(&alp_command->alp_command_fifo, fifo_get_size(&alp_command->alp_command_fifo));
}

static uint8_t application_file_content[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

static alp_status_codes_t on_unhandled_read_cb(const alp_interface_status_t* origin_itf_status, alp_operand_file_data_request_t operand, uint8_t* alp_response) {
    log_print_string("gotten read file of file %d", operand.file_offset.file_id);
    if (operand.file_offset.file_id == APPLICATION_FILE_ID) {
        assert(sizeof(application_file_content) <= (operand.file_offset.offset + operand.requested_data_length));
        log_print_string(
            "gotten read file with offset %i and length %i", operand.file_offset.offset, operand.requested_data_length);
        memcpy(alp_response, application_file_content + operand.file_offset.offset, operand.requested_data_length);
        return SUCCESS;
    }
    return -ENOENT;
}

static alp_init_args_t alp_init_args;

void bootstrap()
{
    log_print_string("Device booted\n");
    d7ap_fs_init();
    d7ap_init();

    alp_init_args.alp_command_completed_cb = &on_alp_command_completed_cb;
    alp_init_args.alp_command_result_cb = &on_alp_command_result_cb;
    alp_init_args.alp_unhandled_read_action_cb = &on_unhandled_read_cb;
    alp_layer_init(&alp_init_args, false);

    memset(query.compare_body, 10, 1);

#if defined USE_HTS221
    hts221_handle = i2c_init(0, 0, 100000, true);
    i2c_acquire(hts221_handle);    
    HTS221_DeActivate(hts221_handle);
    HTS221_Set_BduMode(hts221_handle, HTS221_ENABLE);
    HTS221_Set_Odr(hts221_handle, HTS221_ODR_7HZ);
    HTS221_Activate(hts221_handle);
    i2c_release(hts221_handle);
#endif

    sched_register_task(&execute_sensor_measurement);
    sched_post_task(&execute_sensor_measurement);
}
