/*
 * timer.h
 *
 *  Created on: 26-nov.-2012
 *      Author: Maarten Weyn
 */

#ifndef HAL_TIMER_H_
#define HAL_TIMER_H_

#include <stdbool.h>
#include <stdint.h>

void hal_timer_init();

uint16_t hal_timer_getvalue();

void hal_timer_setvalue(uint16_t next_event);

void hal_timer_enable_interrupt();

void hal_timer_disable_interrupt();


void hal_benchmarking_timer_init();
uint32_t hal_benchmarking_timer_getvalue();
void hal_benchmarking_timer_start();
void hal_benchmarking_timer_stop();

#endif /* HAL_TIMER_H_ */
