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

#include "../queue.h"

// TODO move public timer API & implementation away from HAL because it is not hardware specific.
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

// private function prototypes, those are hardware specific
void _timer_init();

int16_t _timer_getvalue();

void _timer_setvalue(uint16_t next_event);

void _timer_enable_interrupt();

void _timer_disable_interrupt();

#endif /* TIMER_H_ */
