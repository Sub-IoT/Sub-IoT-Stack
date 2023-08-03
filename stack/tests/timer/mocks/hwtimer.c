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

#include "hwtimer.h"

timer_callback_t compare_c;
timer_callback_t overflow_c;
hwtimer_tick_t timer_value;
bool is_overflow_pending;

error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback) { compare_c = compare_callback; overflow_c = overflow_callback; return SUCCESS; }

const hwtimer_info_t* hw_timer_get_info(hwtimer_id_t timer_id) { static const hwtimer_info_t timer_info = {.min_delay_ticks = 0,}; return &timer_info;}

bool hw_timer_is_overflow_pending(hwtimer_id_t id) { return is_overflow_pending; }

error_t hw_timer_cancel(hwtimer_id_t timer_id) { return SUCCESS; }

hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id) { return timer_value; }
 
error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick ) { return SUCCESS; }

void run_compare_c(){compare_c();}
void run_overflow_c(){overflow_c();}
void set_hw_timer_value(hwtimer_tick_t value) { timer_value = value; }
void set_overflow_pending(bool pending) {is_overflow_pending = pending; }