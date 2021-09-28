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


// This examples does not push sensor data to gateway(s) continuously, but instead writes the sensor value to a local file,
// which can then be fetched on request.
// Temperature data is used as a sensor value, when a HTS221 is available, otherwise value 0 is used.
// Contrary to the sensor_push example we are now defining an Access Profile which has a periodic scan automation enabled.
// The sensor will sniff the channel every second for background adhoc synchronization frames, to be able to receive requests from other nodes.

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
#include "compress.h"

#include "d7ap.h"
#include "alp_layer.h"
#include "dae.h"
#include "platform_defs.h"
#include "modules_defs.h"
#include "platform.h"

#ifdef USE_HTS221
  #include "HTS221_Driver.h"
  #include "hwi2c.h"
#endif

#ifdef MODULE_LORAWAN
  #error "sensor_pull app is not compatible with LoRaWAN, so disable MODULE_LORAWAN in cmake"
#endif

#if !defined(USE_SX127X) && !defined(USE_NETDEV_DRIVER)
  #error "background frames are only supported by the sx127x driver for now"
#endif


#define SENSOR_FILE_ID           0x42
#define SENSOR_FILE_SIZE         2
#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC * 10

#ifdef USE_HTS221
  static i2c_handle_t* hts221_handle;
#endif


void execute_sensor_measurement()
{
  static int16_t temperature = 0; // in decicelsius. When there is no sensor, we transmit a rising number

#if defined USE_HTS221
  i2c_acquire(hts221_handle);
  HTS221_Get_Temperature(hts221_handle, &temperature);
  i2c_release(hts221_handle);
#else
  temperature++;
#endif

  temperature = __builtin_bswap16(temperature); // need to store in big endian in fs
  int rc = d7ap_fs_write_file(SENSOR_FILE_ID, 0, (uint8_t*)&temperature, SENSOR_FILE_SIZE, ROOT_AUTH);
  assert(rc == 0);
  temperature = __builtin_bswap16(temperature); // revert to make sure we're working with the right value

  timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);
}

void init_user_files()
{
  // file 0x40: contains our sensor data
  d7ap_fs_file_header_t sensor_file_header = (d7ap_fs_file_header_t) {
      .file_permissions = (file_permission_t) { .user_read = true, .guest_read = true },
      .file_properties.action_protocol_enabled = 0,
      .file_properties.storage_class = FS_STORAGE_PERMANENT,
      .length = SENSOR_FILE_SIZE,
      .allocated_length = SENSOR_FILE_SIZE,
  };

  d7ap_fs_init_file(SENSOR_FILE_ID, &sensor_file_header, NULL);
}

void bootstrap()
{
    log_print_string("Device booted\n");
    
    alp_layer_init(NULL, false);
    
    d7ap_fs_write_dll_conf_active_access_class(0x11); // use scanning AC, visible in d7ap_fs_data.c
    init_user_files();

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
