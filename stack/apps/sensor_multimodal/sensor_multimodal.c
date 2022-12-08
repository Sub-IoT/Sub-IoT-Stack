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


#include "hwleds.h"
#include "hwsystem.h"

#include "scheduler.h"
#include "timer.h"
#include "button.h"
#include "debug.h"
#include "log.h"
#include "led.h"

#include "d7ap.h"
#include "dae.h"
#include "d7ap_fs.h"
#include "alp_layer.h"
#include "lorawan_stack.h"

#include "string.h"

#include "modules_defs.h"
#include "ports.h"

#ifndef MODULE_LORAWAN
  #error "sensor multimodal requires MODULE_LORAWAN=y"
#endif

#define DEBUG_PRINTF(...) 							log_print_string(__VA_ARGS__)

#define SENSOR_FILE_ID                  0x42
#define SENSOR_FILE_SIZE                2
#define SENSOR_INTERVAL_LORAWAN_SEC     TIMER_TICKS_PER_SEC * 60
#define SENSOR_INTERVAL_D7AP_SEC        TIMER_TICKS_PER_SEC * 20

static uint8_t devEui[] = { 0xBE, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x81 };
static uint8_t appEui[] = { 0xBE, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x9F };
static uint8_t appKey[] = { 0x7E, 0xEF, 0x56, 0xEC, 0xDA, 0x1D, 0xD5, 0xA4, 0x70, 0x59, 0xFD, 0x35, 0x9C, 0xE6, 0x80, 0xCD };

typedef enum {
  D7_ACTIVE,
  LORA_ACTIVE
} active_network_t;

static active_network_t current_network;

static alp_init_args_t alp_init_args;

// Define the D7 interface configuration used for sending the ALP command on
static alp_interface_config_d7ap_t d7_itf_cfg = {
  .itf_id = ALP_ITF_ID_D7ASP,
  .d7ap_session_config = {
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
  }
};


// Define the lora interface configuration used for sending the ALP command on
static alp_interface_config_lorawan_otaa_t lora_itf_cfg = { .itf_id = ALP_ITF_ID_LORAWAN_OTAA,
    .lorawan_session_config_otaa
    = { .adr_enabled = true, .application_port = 2, .data_rate = 0, .request_ack = true } };

static void execute_sensor_measurement();

// alp layer will answer if it succeeded or not
void on_alp_command_completed_cb(uint8_t tag_id, bool success) {
  if (success)
    DEBUG_PRINTF("Command completed successfully\n");
  else
    DEBUG_PRINTF("Command failed, no ack received\n");
}

// alp layer will let you know if any noticable events happened or you got an answer including metadata
void on_alp_command_result_cb(alp_command_t* alp_command, alp_interface_status_t* origin_itf_status)
{
    if(origin_itf_status && (origin_itf_status->len > 0)) {
        // gotten answer from D7
        if(origin_itf_status->itf_id == ALP_ITF_ID_D7ASP) {
            d7ap_session_result_t* d7_result = (d7ap_session_result_t*)origin_itf_status->itf_status;
            DEBUG_PRINTF("recv response @ %i dB link budget from:", d7_result->link_budget);
            log_print_data(d7_result->addressee.id, d7ap_addressee_id_length(d7_result->addressee.ctrl.id_type));
        // gotten answer from LoRaWAN
        } else if(origin_itf_status->itf_id == ALP_ITF_ID_LORAWAN_OTAA) {
            lorawan_session_result_t* lora_result = (lorawan_session_result_t*)origin_itf_status->itf_status;
            if(lora_result->error_state == LORAWAN_STACK_JOINED) {
                DEBUG_PRINTF("lora joined, sending message again");
                sched_post_task(&execute_sensor_measurement);
            } else if(lora_result->error_state == LORAWAN_STACK_ERROR_OK) {
                DEBUG_PRINTF("tried to transmit %i times", lora_result->attempts);
            }
        }
    }
    if(fifo_get_size(&alp_command->alp_command_fifo)) {
        DEBUG_PRINTF("response payload:");
        log_print_data(alp_command->alp_command, fifo_get_size(&alp_command->alp_command_fifo));
        fifo_skip(&alp_command->alp_command_fifo, fifo_get_size(&alp_command->alp_command_fifo));
    }
}

static uint8_t transmit(uint8_t* alp, uint16_t len) {
  DEBUG_PRINTF("sending over %s", (current_network == D7_ACTIVE) ? "D7" : "LoRa");
  alp_command_t* command = alp_layer_command_alloc(true, true);
  if(!command)
      log_print_error_string("could not allocate command, make sure there are enough commands active (using cmake "
                             "option MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT)");

  alp_interface_config_t* itf_cfg = (current_network == D7_ACTIVE) ? (alp_interface_config_t*) &d7_itf_cfg : (alp_interface_config_t*) &lora_itf_cfg;
  uint8_t itf_cfg_len = (current_network == D7_ACTIVE) ? d7ap_session_config_length(&d7_itf_cfg.d7ap_session_config) : sizeof(lora_itf_cfg.lorawan_session_config_otaa);

  alp_append_forward_action(command, itf_cfg, itf_cfg_len);

  fifo_put(&command->alp_command_fifo, alp, len);

  alp_layer_process(command);

  return 0;
}

static void on_button_pressed(button_id_t button_id) {
  // alp layer will handle the switch
  DEBUG_PRINTF("Switching from %s", (current_network == D7_ACTIVE) ? "D7 to LoRa" : "LoRa to D7");

  current_network = !current_network;
}

static void execute_sensor_measurement()
{
  // first get the sensor reading ...
  static int16_t temperature = 0; // in decicelsius. When there is no sensor, we transmit a rising number

#if defined USE_HTS221
  HTS221_Get_Temperature(hts221_handle, &temperature);
#else
  temperature++;
#endif

  // Generate ALP command. We do this manually for now (until we have an API for this).
  // We will be sending a return file data action, without a preceding file read request.
  // This is an unsolicited message, where we push the sensor data to the gateway(s).
  // Please refer to the spec for the format

  uint8_t alp_command[4 + SENSOR_FILE_SIZE] = {
    // ALP Control byte
    ALP_OP_RETURN_FILE_DATA,
    // File Data Request operand:
    SENSOR_FILE_ID, // the file ID
    0, // offset in file
    SENSOR_FILE_SIZE // data length
    // the sensor data, see below
  };

  temperature = __builtin_bswap16(temperature); // convert to big endian before transmission
  memcpy(alp_command + 4, (uint8_t*)&temperature, SENSOR_FILE_SIZE);
  temperature = __builtin_bswap16(temperature); // convert to big endian before transmission

  transmit(alp_command, sizeof(alp_command));

  timer_tick_t delay = (current_network == D7_ACTIVE) ? SENSOR_INTERVAL_D7AP_SEC : SENSOR_INTERVAL_LORAWAN_SEC;

  DEBUG_PRINTF("sensor measurement executed with temperature %i, now waiting %i ticks to measure again", temperature, delay);
  timer_post_task_delay(&execute_sensor_measurement, delay);
}

void bootstrap() {
  DEBUG_PRINTF("Device booted\n");

  alp_init_args.alp_command_completed_cb = &on_alp_command_completed_cb;
  alp_init_args.alp_command_result_cb = &on_alp_command_result_cb;
  alp_layer_init(&alp_init_args, false);

  // this can also be specified in d7ap_fs_data.c but is here for clarity
  d7ap_fs_write_file(D7A_FILE_UID_FILE_ID, 0, devEui, D7A_FILE_UID_SIZE, ROOT_AUTH);
  d7ap_fs_write_file(USER_FILE_LORAWAN_KEYS_FILE_ID, 0, appEui, 8, ROOT_AUTH);
  d7ap_fs_write_file(USER_FILE_LORAWAN_KEYS_FILE_ID, 8, appKey, 16, ROOT_AUTH);

  ubutton_register_callback(0, &on_button_pressed);

  sched_register_task(&execute_sensor_measurement);
  sched_post_task(&execute_sensor_measurement);
}
