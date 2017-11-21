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

typedef struct
{
  /*point to the LoRa App data buffer*/
  uint8_t* Buff;
  /*LoRa App data buffer size*/
  uint8_t BuffSize;
  /*Port on which the LoRa App is data is sent/ received*/
  uint8_t Port;

} lora_AppData_t;

typedef void (*lora_rx_callback_t) (lora_AppData_t *AppData);

void lorawan_stack_init(uint8_t devEUI[8], uint8_t appEUI[8], uint8_t appKey[16], lora_rx_callback_t cb);
void lorawan_stack_tick();
bool lorawan_stack_send(uint8_t* payload, uint8_t length, uint8_t app_port, bool request_ack);

#endif //LORAWAN_STACK_H

