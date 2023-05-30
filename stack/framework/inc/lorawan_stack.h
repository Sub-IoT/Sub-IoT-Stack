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

#ifndef LORAWAN_STACK_H
#define LORAWAN_STACK_H

#include "types.h"

typedef struct
{
  /*point to the LoRa App data buffer*/
  uint8_t* Buff;
  /*LoRa App data buffer size*/
  uint8_t BuffSize;
  /*Port on which the LoRa App is data is sent/ received*/
  uint8_t Port;

} lorawan_AppData_t;

typedef enum {
  LORAWAN_STACK_ERROR_OK,
  LORAWAN_STACK_ERROR_NOT_JOINED,
  LORAWAN_STACK_ERROR_TX_NOT_POSSIBLE,
  LORAWAN_STACK_ERROR_UNKNOWN,
  LORAWAN_STACK_ERROR_NACK,
  LORAWAN_STACK_JOIN_FAILED,
  LORAWAN_STACK_DUTY_CYCLE_DELAY,
  LORAWAN_STACK_RETRY_TRANSMISSION,
  LORAWAN_STACK_JOINED,
  LORAWAN_STACK_ALREADY_JOINING,
  LORAWAN_STACK_ALREADY_TRANSMITTING,
  LORAWAN_STACK_ERROR_NOT_INITED
} lorawan_stack_status_t;

typedef struct __attribute__((__packed__)) {
    uint8_t attempts;
    lorawan_stack_status_t error_state;
    uint16_t duty_cycle_wait_time;
} lorawan_session_result_t;

typedef struct __attribute__((__packed__)) {
    union {
      uint8_t raw;
      struct {
        uint8_t _rfu : 1;
        bool request_ack : 1;
        bool adr_enabled : 1;
        uint8_t __rfu : 5;
      };
    };
    uint8_t application_port;
    uint8_t data_rate;
} lorawan_session_config_otaa_t;

// override alp_interface_config_t
typedef struct {
    uint8_t itf_id;
    lorawan_session_config_otaa_t lorawan_session_config_otaa;
} __attribute__ ((__packed__)) alp_interface_config_lorawan_otaa_t;

typedef void (*lorawan_rx_callback_t)(lorawan_AppData_t *AppData);
typedef void (*join_completed_callback_t)(bool success);
typedef void (*lorawan_tx_completed_callback_t)(lorawan_stack_status_t status, uint8_t retries);
typedef void (*lorawan_duty_cycle_delay_callback_t)(uint32_t delay, uint8_t attempt );
typedef void (*lorawan_join_attempt_callback_t)(uint8_t join_attempt_number);
typedef void (*lorawan_status_callback_t)(lorawan_stack_status_t status, uint8_t attempt);

lorawan_stack_status_t lorawan_otaa_is_joined(lorawan_session_config_otaa_t* lorawan_session_config);
error_t lorawan_stack_init_otaa();

void lorawan_stack_deinit(void);
bool lorawan_stack_join(void);
void lorawan_register_cbs(lorawan_rx_callback_t  lorawan_rx_cb, lorawan_tx_completed_callback_t lorawan_tx_cb, lorawan_status_callback_t lorawan_status_cb );
lorawan_stack_status_t lorawan_stack_send(uint8_t* payload, uint8_t length, uint8_t app_port, bool request_ack);
uint16_t lorawan_get_duty_cycle_delay();
#endif //LORAWAN_STACK_H

