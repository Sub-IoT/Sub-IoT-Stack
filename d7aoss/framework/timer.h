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
uint16_t get_counter_value();

void timer_enable_interrupt();
void timer_disable_interrupt();

#endif /* TIMER_H_ */
