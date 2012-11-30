/*
 * timer.h
 *
 *  Created on: 26-nov.-2012
 *      Author: Maarten Weyn
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "types.h"

typedef struct
{
	u16 next_event;
	void (*f) (void*);

} timer_event;

void timer_init();

void timer_add_event(timer_event* event);

#endif /* TIMER_H_ */
