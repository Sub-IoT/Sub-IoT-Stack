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

#include <stdio.h>
#include <stdlib.h>

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


// This examples pushes sensor data to gateway(s) by writing it to a local file, which is configured to trigger a file action (using D7AActP)
// which results in reading this file and sending the result to the D7 interface. The D7 session is configured not to request ACKs.
// Temperature data is used as a sensor value, when a HTS221 is available, otherwise value 0 is used.

#define SENSOR_FILE_ID           0x40
#define SENSOR_FILE_SIZE         2
#define ACTION_FILE_ID           0x41
#define INTERFACE_FILE_ID        0x42

#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC * 30

#ifdef USE_HTS221
  static i2c_handle_t* hts221_handle;
#endif

static blockdevice_stm32_eeprom_t systemfiles_eeprom_blockdevice;

void execute_sensor_measurement()
{
  int16_t temperature = 0; // in decicelsius. When there is no sensor, we just transmit 0 degrees

#if defined USE_HTS221
  HTS221_Get_Temperature(hts221_handle, &temperature);
#endif

  temperature = __builtin_bswap16(temperature); // need to store in big endian in fs
  d7ap_fs_write_file(SENSOR_FILE_ID, 0, (uint8_t*)&temperature, SENSOR_FILE_SIZE);

  log_print_string("temp %i dC", temperature);
  timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);
}

void init_user_files()
{
  // configure file notification using D7AActP: changes made to file SENSOR_FILE_ID will result in the action in file ACTION_FILE_ID
  // being executed of which the results will transmitted to the interface defined in file INTERFACE_FILE_ID

  // first generate ALP command for the action file. We do this manually for now (until we have an API for this).
  // Please refer to the spec for the format
  uint8_t alp_command[4] = {
    // ALP Control byte
    ALP_OP_READ_FILE_DATA,
    // File Data Request operand:
    SENSOR_FILE_ID, // the file ID
    0, // offset in file
    SENSOR_FILE_SIZE // requested data length
  };

  fs_file_header_t action_file_header = (fs_file_header_t){
    .file_properties.action_protocol_enabled = 0,
    .file_properties.storage_class = FS_STORAGE_PERMANENT,
    .file_permissions = 0, // TODO
    .length = sizeof(alp_command),
    .allocated_length = sizeof(alp_command),
  };

  d7ap_fs_init_file(ACTION_FILE_ID, &action_file_header, alp_command);

  // define the D7 interface configuration used for sending the result of above ALP command on
  d7ap_session_config_t session_config = {
    .qos = {
      .qos_resp_mode = SESSION_RESP_MODE_NO,
      .qos_retry_mode = SESSION_RETRY_MODE_NO,
      .qos_stop_on_error = false,
      .qos_record = false
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

  d7ap_fs_init_file_with_d7asp_interface_config(INTERFACE_FILE_ID, &session_config);

  // finally, register the sensor file, configured to use D7AActP
  fs_file_header_t file_header = (fs_file_header_t){
    .file_properties.action_protocol_enabled = 1,
    .file_properties.action_condition = ALP_ACT_COND_WRITE,
    .file_properties.storage_class = FS_STORAGE_VOLATILE,
    .file_permissions = 0, // TODO
    .alp_cmd_file_id = ACTION_FILE_ID,
    .interface_file_id = INTERFACE_FILE_ID,
    .length = SENSOR_FILE_SIZE
  };

  d7ap_fs_init_file(SENSOR_FILE_ID, &file_header, NULL);
}

void bootstrap()
{
    log_print_string("Device booted\n");

    systemfiles_eeprom_blockdevice = (blockdevice_stm32_eeprom_t){
      .base.driver = &blockdevice_driver_stm32_eeprom,
    };

    blockdevice_init((blockdevice_t*)&systemfiles_eeprom_blockdevice);
    d7ap_init((blockdevice_t*)&systemfiles_eeprom_blockdevice);
    alp_layer_init(NULL, false);

    init_user_files();

#if defined USE_HTS221
    hts221_handle = i2c_init(0, 0, 100000);
    HTS221_DeActivate(hts221_handle);
    HTS221_Set_BduMode(hts221_handle, HTS221_ENABLE);
    HTS221_Set_Odr(hts221_handle, HTS221_ODR_7HZ);
    HTS221_Activate(hts221_handle);
#endif

    sched_register_task(&execute_sensor_measurement);
    timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);
}
