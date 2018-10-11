/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2017 University of Antwerp
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

#ifndef LORAWAN_STACK_H
#define LORAWAN_STACK_H

#include "types.h"
#include "LoRaMac.h"

typedef struct
{
  /*point to the LoRa App data buffer*/
  uint8_t* Buff;
  /*LoRa App data buffer size*/
  uint8_t BuffSize;
  /*Port on which the LoRa App is data is sent/ received*/
  uint8_t Port;

} lora_AppData_t;

typedef enum {
  LORAWAN_STACK_ERROR_OK,
  LORAWAN_STACK_ERROR_NOT_JOINED,
  LORAWAN_STACK_ERROR_TX_NOT_POSSIBLE,
  LORAWAN_STACK_ERROR_UNKNOWN,
} lorawan_stack_error_t;

typedef enum {
  ABP,
  OTAA,
} activationMethod_t;

typedef struct {
    uint8_t devEUI[8];
    uint8_t appEUI[8];
    uint8_t appKey[16];
    activationMethod_t activationMethod;
    uint8_t nwkSKey[16] ;
    uint8_t appSKey[16] ;
    uint32_t devAddr;
    uint32_t network_id;
    bool request_ack;
    uint8_t application_port;
} lora_session_config_t;

typedef void (*lora_rx_callback_t)(lora_AppData_t *AppData);
typedef void (*join_completed_callback_t)(bool success,uint8_t app_port2,bool request_ack2);
typedef void (*lora_tx_callback_t)(McpsConfirm_t *McpsConfirm);


void lorawan_stack_init(uint8_t devEUI[8], uint8_t appEUI[8], uint8_t appKey[16], join_completed_callback_t join_failed_cb, lora_rx_callback_t cb);
bool refresh_lorawan_session_config(lora_session_config_t* lora_session_config);
void lorawan_stack_init2(lora_session_config_t* activationKeys);
void lorawan_stack_deinit();
bool lorawan_stack_join();
void lora_register_cbs(lora_rx_callback_t  lora_rx_cb,lora_tx_callback_t lora_tx_cb,join_completed_callback_t join_completed_cb);
lorawan_stack_error_t lorawan_stack_send(uint8_t* payload, uint8_t length, uint8_t app_port, bool request_ack);

#endif //LORAWAN_STACK_H

