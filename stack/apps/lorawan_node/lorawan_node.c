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

#include "log.h"

#include "d7ap_fs.h"
#include "lorawan_stack.h"
#include "scheduler.h"
#include "string.h"
#include "timer.h"

#include "modules_defs.h"

#ifndef MODULE_LORAWAN
  #error enable cmake option MODULE_LORAWAN to run this example
#endif

#ifdef MODULE_ALP
  #error cmake option MODULE_ALP has to be disabled as we are not using alp in this example
#endif

// TODO
lorawan_session_config_otaa_t lorawan_session_config
    = { .adr_enabled = true, .application_port = 2, .data_rate = 0, .request_ack = true };
static uint8_t devEui[] = { 0xBE, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x81 };
static uint8_t appEui[] = { 0xBE, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x9F };
static uint8_t appKey[] = { 0x7E, 0xEF, 0x56, 0xEC, 0xDA, 0x1D, 0xD5, 0xA4, 0x70, 0x59, 0xFD, 0x35, 0x9C, 0xE6, 0x80, 0xCD };


static void read_sensor_task()
{
  static uint8_t msg_counter = 0;
  log_print_string("read sensor");
  lorawan_stack_status_t e = lorawan_stack_send(&msg_counter, 1, 2, false);
  if(e == LORAWAN_STACK_ERROR_OK) {
    log_print_string("TX ok (%i)", msg_counter);
  } else {
    log_print_string("TX failed");
  }

  msg_counter++;
  timer_post_task_delay(&read_sensor_task, 60 * TIMER_TICKS_PER_SEC);
}

void lorawan_status_cb(lorawan_stack_status_t status, uint8_t attempt)
{
  if(status == LORAWAN_STACK_JOIN_FAILED) {
    log_print_string("join failed");
    lorawan_otaa_is_joined(&lorawan_session_config);
  } else if(status == LORAWAN_STACK_JOINED){
    log_print_string("join succeeded");
    sched_post_task(&read_sensor_task);
  }
}
void lorawan_rx(lorawan_AppData_t *AppData)
{
   log_print_string("RECEIVED DATA"); //TODO
}
void lorawan_tx(lorawan_stack_status_t status, uint8_t attempt)
{
   log_print_string("TRANSMITTED DATA"); //TODO
}

void bootstrap()
{
    log_print_string("Device booted\n");
    d7ap_fs_init();

    
  // this can also be specified in d7ap_fs_data.c but is here for clarity
    d7ap_fs_write_file(D7A_FILE_UID_FILE_ID, 0, devEui, D7A_FILE_UID_SIZE, ROOT_AUTH);
    d7ap_fs_write_file(USER_FILE_LORAWAN_KEYS_FILE_ID, 0, appEui, 8, ROOT_AUTH);
    d7ap_fs_write_file(USER_FILE_LORAWAN_KEYS_FILE_ID, 8, appKey, 16, ROOT_AUTH);
    lorawan_register_cbs(lorawan_rx, lorawan_tx, lorawan_status_cb);
    lorawan_stack_init_otaa();

    lorawan_otaa_is_joined(&lorawan_session_config);

    sched_register_task(&read_sensor_task);
}