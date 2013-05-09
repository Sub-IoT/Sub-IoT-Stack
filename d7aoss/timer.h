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

typedef struct
{
	uint16_t next_event;
	void (*f) (void*);

} timer_event;

void timer_init();

bool timer_add_event(timer_event* event);
uint16_t get_counter_value();
#endif /* TIMER_H_ */
