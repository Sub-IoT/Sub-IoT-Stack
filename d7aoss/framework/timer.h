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
    void (*f) (void);
    int32_t next_event;
} timer_event;

void            timer_init                  ( void );
void            timer_completed             ( void );
bool            timer_add_event             ( timer_event* event );
uint32_t        timer_get_counter_value     ( void );

static uint8_t  timer_get_next_event        ( void );
static void     timer_configure_next_event  ( void );
static bool     timer_add_event_in_stack    ( timer_event new_event );
static bool     timer_update_stack          ( void );

static void     timer_wait_done             ( void );
void            timer_wait_ms               ( uint32_t ms );

void benchmarking_timer_init();
uint32_t benchmarking_timer_getvalue();
void benchmarking_timer_start();
void benchmarking_timer_stop();

#endif /* TIMER_H_ */
