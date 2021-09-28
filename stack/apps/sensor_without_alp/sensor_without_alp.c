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


// This example is similar to sensor_push except that it does not use the ALP layer.
// Note that this only exist as an example on how to build without ALP layer, allowing you to plug other upper layer on top.
// Other examples like the gateway are all based on ALP and thus not compatible with this example.
// It will also be in RX mode continuously allowing to flash on 2 devices which will receive each other


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
#include "dae.h"

#include "platform.h"

#include "modules_defs.h"

#ifdef MODULE_ALP
#error "This app should be build with MODULE_ALP=n"
#endif


#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC * 30


// Define the D7 interface configuration used for sending payload on
static d7ap_session_config_t d7ap_session_config = (d7ap_session_config_t){
    .qos = {
        .qos_resp_mode = SESSION_RESP_MODE_PREFERRED,
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
};

void on_receive(uint16_t trans_id, uint8_t* payload, uint8_t len, d7ap_session_result_t result);
void on_transmitted(uint16_t trans_id, error_t error);
bool on_unsolicited_response(uint8_t* payload, uint8_t len, d7ap_session_result_t result, bool response_expected);

d7ap_resource_desc_t callbacks = {
    .receive_cb = &on_receive,
    .transmitted_cb = &on_transmitted,
    .unsolicited_cb = &on_unsolicited_response
};

uint8_t d7_client_id;

void execute_sensor_measurement()
{
  // first get the sensor reading ...
  int16_t temperature = 0; // in decicelsius. When there is no sensor, we just transmit 0 degrees

#if defined USE_HTS221
  HTS221_Get_Temperature(hts221_handle, &temperature);
#endif

  temperature = __builtin_bswap16(temperature); // convert to big endian before transmission

  uint16_t trans_id;
  d7ap_send(d7_client_id, &d7ap_session_config, (uint8_t*)&temperature, sizeof(temperature), 0, &trans_id);
}

void on_receive(uint16_t trans_id, uint8_t* payload, uint8_t len, d7ap_session_result_t result)
{
    log_print_string("Received\n");
}

void on_transmitted(uint16_t trans_id, error_t error)
{
    log_print_string("Transmitted (with error: %i)\n", error);
    timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);
}

bool on_unsolicited_response(uint8_t* payload, uint8_t len, d7ap_session_result_t result, bool response_expected)
{
    log_print_string("Unsolicited response received\n");
    return false;
}

void bootstrap()
{
    log_print_string("Device booted\n");

    d7ap_fs_init();
    d7ap_init();
    d7_client_id = d7ap_register(&callbacks);

    d7ap_set_access_class(0x01); // continuous FS scan, visible in d7ap_fs_data.c

    sched_register_task(&execute_sensor_measurement);
    sched_post_task(&execute_sensor_measurement);
}
