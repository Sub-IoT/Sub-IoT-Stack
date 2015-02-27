/*

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
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
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *
 */

#ifndef TIMER_H_
#define TIMER_H_

//#define ENABLE_BENCHMARKING_TIMER

#include <stdbool.h>
#include <stdint.h>

#include "queue.h"

typedef struct
{
    void (*f) (void);
    uint16_t next_event;
} timer_event;

static timer_event event_array[20];
static queue_t event_queue;
static bool started;

void timer_init(void);

void timer_completed(void);

bool timer_add_event(timer_event* event);
uint16_t timer_get_counter_value(void);


void timer_wait_done(void);
void timer_wait_ms(uint16_t ms);

void benchmarking_timer_init();
uint32_t benchmarking_timer_getvalue();
void benchmarking_timer_start();
void benchmarking_timer_stop();

#endif /* TIMER_H_ */
