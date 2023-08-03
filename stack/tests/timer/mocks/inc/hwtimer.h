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

#ifndef __HWTIMER_MOCK_H
#define __HWTIMER_MOCK_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "errors.h"

typedef struct {
    uint8_t min_delay_ticks;
} hwtimer_info_t;

typedef void (*timer_callback_t)(void);
typedef uint8_t hwtimer_id_t;
typedef uint16_t hwtimer_tick_t;

void run_compare_c();
void run_overflow_c();
void set_hw_timer_value(hwtimer_tick_t value);
void set_overflow_pending(bool pending);
//hwtimer_info_t timer_info;

error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback);
const hwtimer_info_t* hw_timer_get_info(hwtimer_id_t timer_id);
bool hw_timer_is_overflow_pending(hwtimer_id_t id);
error_t hw_timer_cancel(hwtimer_id_t timer_id);
hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id);
error_t hw_timer_schedule_delay(hwtimer_id_t timer_id, hwtimer_tick_t delay);
error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick );

#endif //__HWTIMER_MOCK_H