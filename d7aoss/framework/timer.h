/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
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
    uint16_t next_event;
    void (*f) (void);
} timer_event;

static timer_event event_array[20];
static queue_t event_queue;
static bool started;

void timer_init();

void timer_completed();

bool timer_add_event(timer_event* event);
uint16_t timer_get_counter_value();



void benchmarking_timer_init();
uint32_t benchmarking_timer_getvalue();
void benchmarking_timer_start();
void benchmarking_timer_stop();

#endif /* TIMER_H_ */
