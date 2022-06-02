/*
 * Copyright (c) 2015-2022 University of Antwerp, Aloxy NV.
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

#ifndef HWEXTERNALBUSUNCOUPLER_H_
#define HWEXTERNALBUSUNCOUPLER_H_

#include "hwgpio.h"

typedef struct uncoupler_handle uncoupler_handle_t;

typedef struct 
{
    void (*uncoupler_set)(uncoupler_handle_t* handle, bool enable);
} uncoupler_driver_t;


struct uncoupler_handle
{
    uncoupler_driver_t *driver;
    uint32_t priv_data[2];
};

void uncoupler_init(uncoupler_handle_t* handle, pin_id_t pin, bool is_active_low);

#endif // HWEXTERNALBUSUNCOUPLER_H_
