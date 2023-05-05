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

#ifndef LORAWAN_FUOTA_DEMO_H
#define LORAWAN_FUOTA_DEMO_H

#include "types.h"
#include "LoRaMac.h"

bool lorawan_fuota_demo_otaa_is_joined(bool adr_enabled, uint8_t data_rate);

error_t lorawan_fuota_demo_init(LoRaMacRegion_t region, uint8_t* appKey, uint8_t* payload_buffer);

void lorawan_fuota_demo_deinit();

#endif //LORAWAN_FUOTA_DEMO_H