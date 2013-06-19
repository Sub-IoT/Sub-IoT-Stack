/*
 * timer.h
 *
 *  Created on: 26-nov.-2012
 *      Author: Maarten Weyn
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdbool.h>
#include <stdint.h>

#include "queue.h"

typedef struct
{
	uint16_t next_event;
	void (*f) (void*);

} timer_event;

static timer_event event_array[20];
static queue event_queue;
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
