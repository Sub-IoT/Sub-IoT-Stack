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

void hal_timer_init();

int16_t hal_timer_getvalue();

void hal_timer_setvalue(uint16_t next_event);

void hal_timer_enable_interrupt();

void hal_timer_disable_interrupt();

#endif /* TIMER_H_ */
