/*! \file timer.h
 *

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
 * \author maarten.weyn@uantwerpen.be
 *
 */

#ifndef HAL_TIMER_H_
#define HAL_TIMER_H_

#include <stdbool.h>
#include <stdint.h>
//**********************************************/
//Initialize and start the timer                          /
//**********************************************/
void hal_timer_init();

//**********************************************/
//Get the nuber of ticks from the start of the timer                      /
//**********************************************/
uint16_t hal_timer_getvalue(); // TODO return 32 bit for all platforms?

//**********************************************/
//Initialize the timer interrupt value                          /
//**********************************************/
void hal_timer_setvalue(uint16_t next_event);

//**********************************************/
//Enable the interrupt on the timer                          /
//**********************************************/
void hal_timer_enable_interrupt();

//**********************************************/
//Disable the interrupt on the timer                          /
//**********************************************/
void hal_timer_disable_interrupt();

void hal_timer_counter_reset();
void hal_timer_clear_interrupt( void );

void hal_benchmarking_timer_init();
uint32_t hal_benchmarking_timer_getvalue();
void hal_benchmarking_timer_start();
void hal_benchmarking_timer_stop();

#endif /* HAL_TIMER_H_ */
