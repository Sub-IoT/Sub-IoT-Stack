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


// This examples does not push sensor data to gateway(s) continuously, but instead writes the sensor value to a local file,
// which can then be fetched on request.
// Temperature data is used as a sensor value, when a HTS221 is available, otherwise value 0 is used.
// Contrary to the sensor_push example we are now defining an Access Profile which has a periodic scan automation enabled.
// The sensor will sniff the channel every second for background adhoc synchronization frames, to be able to receive requests from other nodes.


#include "hwleds.h"
#include "hwsystem.h"
#include "scheduler.h"
#include "timer.h"
#include "assert.h"

#include <stdio.h>
#include <stdlib.h>

#include "hwlcd.h"
#include "d7ap_stack.h"
#include "fs.h"
#include "log.h"
#include "compress.h"

#ifdef USE_HTS221
  #include "HTS221_Driver.h"
  #include "hwi2c.h"
#endif

#ifndef USE_SX127X
  #error "background frames are only supported by the sx127x driver for now"
#endif


#define SENSOR_FILE_ID           0x40
#define SENSOR_FILE_SIZE         2
#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC * 10

#ifdef USE_HTS221
  static i2c_handle_t* hts221_handle;
#endif

void execute_sensor_measurement()
{
  // first get the sensor reading ...
  int16_t temperature = 0; // in decicelsius. When there is no sensor, we just transmit 0 degrees

#if defined USE_HTS221
  HTS221_Get_Temperature(hts221_handle, &temperature);
#endif

  // Generate ALP command. We do this manually for now (until we have an API for this).
  // We will write to a local file

  uint8_t alp_command[4 + sizeof(temperature)] = {
    // ALP Control byte
    ALP_OP_WRITE_FILE_DATA,
    // File Data Request operand:
    SENSOR_FILE_ID, // the file ID
    0, // offset in file
    SENSOR_FILE_SIZE // data length
    // the sensor data, see below
  };

  memcpy(alp_command + 4, (uint8_t*)&temperature, sizeof(temperature));

  uint8_t resp = 0;
  alp_process_command(alp_command, sizeof(alp_command), alp_command, &resp, ALP_CMD_ORIGIN_APP);

  timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);
}

void init_user_files()
{
  // file 0x40: contains our sensor data
  fs_file_header_t file_header = (fs_file_header_t){
      .file_properties.action_protocol_enabled = 0,
      .file_permissions = 0, // TODO
      .length = SENSOR_FILE_SIZE,
  };

  fs_init_file(SENSOR_FILE_ID, &file_header, NULL);
}

void bootstrap()
{
    log_print_string("Device booted\n");

    dae_access_profile_t access_classes[2] = {
        {
            // AC used for pushing data to the GW, no scanning
            .channel_header = {
                .ch_coding = PHY_CODING_PN9,
                .ch_class = PHY_CLASS_NORMAL_RATE,
                .ch_freq_band = PHY_BAND_868
            },
            .subprofiles[0] = {
                .subband_bitmap = 0x00, // void scan automation channel list
                .scan_automation_period = 0,
            },
            .subbands[0] = (subband_t){
                .channel_index_start = 0,
                .channel_index_end = 0,
                .eirp = 10,
                .cca = 86,
                .duty = 0,
            }
        },
        {
            // AC used for scanning for BG request every second
            .channel_header = {
                .ch_coding = PHY_CODING_PN9,
                .ch_class = PHY_CLASS_NORMAL_RATE,
                .ch_freq_band = PHY_BAND_868
            },
            .subprofiles[0] = {
              .subband_bitmap = 0x01,
              .scan_automation_period = compress_data(1024, true),
            },
            .subbands[0] = (subband_t){
                .channel_index_start = 0,
                .channel_index_end = 0,
                .eirp = 10,
                .cca = 86,
                .duty = 0,
            }
        }
    };

    fs_init_args_t fs_init_args = (fs_init_args_t){
        .access_profiles_count = 2,
        .access_profiles = access_classes,
        .access_class = 0x11, // use scanning AC
        .fs_user_files_init_cb = &init_user_files
    };

    d7ap_stack_init(&fs_init_args, NULL, false, NULL);

#if defined USE_HTS221
    hts221_handle = i2c_init(0, 0);
    HTS221_DeActivate(hts221_handle);
    HTS221_Set_BduMode(hts221_handle, HTS221_ENABLE);
    HTS221_Set_Odr(hts221_handle, HTS221_ODR_7HZ);
    HTS221_Activate(hts221_handle);
#endif

    sched_register_task(&execute_sensor_measurement);
    timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);
}
