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

#include "stm32_common_eeprom.h"

#ifdef USE_HTS221
  #include "HTS221_Driver.h"
  #include "hwi2c.h"
#endif

#define SENSOR_FILE_ID           0x40
#define SENSOR_FILE_SIZE         2
#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC * 30

#ifdef USE_HTS221
  static i2c_handle_t* hts221_handle;
#endif

static blockdevice_stm32_eeprom_t systemfiles_eeprom_blockdevice;

// Define the D7 interface configuration used for sending the ALP command on
static d7ap_session_config_t session_config = {
    .qos = {
        .qos_resp_mode = SESSION_RESP_MODE_PREFERRED,
        .qos_retry_mode = SESSION_RETRY_MODE_NO,
        .qos_stop_on_error       = false,
        .qos_record              = false
    },
    .dormant_timeout = 0,
    .addressee = {
        .ctrl = {
            .nls_method = AES_NONE,
            .id_type = ID_TYPE_NOID,
        },
        .access_class = 0x01,
        .id = 0
    }
};



void execute_sensor_measurement()
{
  // first get the sensor reading ...
  int16_t temperature = 0; // in decicelsius. When there is no sensor, we just transmit 0 degrees

#if defined USE_HTS221
  HTS221_Get_Temperature(hts221_handle, &temperature);
#endif

  temperature = __builtin_bswap16(temperature); // convert to big endian before transmission

  // Generate ALP command.
  // We will be sending a return file data action, without a preceding file read request.
  // This is an unsolicited message, where we push the sensor data to the gateway(s).

  // allocate a buffer and fifo to store the command
  uint8_t alp_command[128];
  fifo_t alp_command_fifo;
  fifo_init(&alp_command_fifo, alp_command, sizeof(alp_command));

  // add the return file data action
  alp_append_return_file_data_action(&alp_command_fifo, SENSOR_FILE_ID, 0, SENSOR_FILE_SIZE, (uint8_t*)&temperature);

  // and execute this
  alp_layer_execute_command_over_d7a(alp_command, fifo_get_size(&alp_command_fifo), &session_config);
}

void on_alp_command_completed_cb(uint8_t tag_id, bool success)
{
    if(success)
      log_print_string("Command completed successfully");
    else
      log_print_string("Command failed, no ack received");

    // reschedule sensor measurement
    timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);
}

void on_alp_command_result_cb(d7ap_session_result_t result, uint8_t* payload, uint8_t payload_length)
{
    log_print_string("recv response @ %i dB link budget from:", result.link_budget);
    log_print_data(result.addressee.id, 8);
}

static alp_init_args_t alp_init_args;

void bootstrap()
{

    log_print_string("Device booted\n");

    // TODO remove
//    fs_init_args_t fs_init_args = (fs_init_args_t){
//        .fs_d7aactp_cb = &alp_layer_process_d7aactp,
//    };

    systemfiles_eeprom_blockdevice = (blockdevice_stm32_eeprom_t){
      .base.driver = &blockdevice_driver_stm32_eeprom,
    };

    blockdevice_init((blockdevice_t*)&systemfiles_eeprom_blockdevice);

    d7ap_init((blockdevice_t*)&systemfiles_eeprom_blockdevice);

    alp_init_args.alp_command_completed_cb = &on_alp_command_completed_cb;
    alp_init_args.alp_command_result_cb = &on_alp_command_result_cb;
    alp_layer_init(&alp_init_args, false);

#if defined USE_HTS221
    hts221_handle = i2c_init(0, 0, 100000);
    HTS221_DeActivate(hts221_handle);
    HTS221_Set_BduMode(hts221_handle, HTS221_ENABLE);
    HTS221_Set_Odr(hts221_handle, HTS221_ODR_7HZ);
    HTS221_Activate(hts221_handle);
#endif

    sched_register_task(&execute_sensor_measurement);
    sched_post_task(&execute_sensor_measurement);
}
