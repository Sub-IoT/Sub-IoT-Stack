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


// This examples pushes sensor data to LoRaWAN gateway(s) by manually constructing an ALP command with a file read result action
// (unsolicited message). All LoRaWAN frames are sent in confirmed mode (request a downlink ACK). All received ACKs are printed.
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

#include "aes.h"

#include "platform.h"

#include "lorawan_stack.h"

#ifdef USE_HTS221
  #include "HTS221_Driver.h"
  #include "hwi2c.h"
#endif

#define SENSOR_FILE_ID           0x42
#define SENSOR_FILE_SIZE         2
#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC * 60

#ifdef USE_HTS221
  static i2c_handle_t* hts221_handle;
#endif

// Define the LoRaWAN interface configuration used for sending the ALP command on
static alp_interface_config_lorawan_otaa_t itf_config = (alp_interface_config_lorawan_otaa_t){
  .itf_id = ALP_ITF_ID_LORAWAN_OTAA,
  .lorawan_session_config_otaa = {
    .request_ack = true,
    .adr_enabled = true,
    .application_port = 2,
    .data_rate = 0
  }
};

static uint8_t devEui[] = { 0xBE, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x81 };
static uint8_t appEui[] = { 0xBE, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x9F };
static uint8_t appKey[] = { 0x7E, 0xEF, 0x56, 0xEC, 0xDA, 0x1D, 0xD5, 0xA4, 0x70, 0x59, 0xFD, 0x35, 0x9C, 0xE6, 0x80, 0xCD };

void execute_sensor_measurement()
{
  log_print_string("execute sensor measurement");
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
  
  // forward to the LoRaWAN interface
  alp_append_forward_action(command, (alp_interface_config_t*)&itf_config, sizeof(itf_config.lorawan_session_config_otaa));

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

static alp_init_args_t alp_init_args;

void bootstrap()
{
    log_print_string("Device booted\n");

    alp_init_args.alp_command_completed_cb = &on_alp_command_completed_cb;
    alp_layer_init(&alp_init_args, false);

    // this can also be specified in d7ap_fs_data.c but is here for clarity
    d7ap_fs_write_file(D7A_FILE_UID_FILE_ID, 0, devEui, D7A_FILE_UID_SIZE, ROOT_AUTH);
    d7ap_fs_write_file(USER_FILE_LORAWAN_KEYS_FILE_ID, 0, appEui, 8, ROOT_AUTH);
    d7ap_fs_write_file(USER_FILE_LORAWAN_KEYS_FILE_ID, 8, appKey, 16, ROOT_AUTH);

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
