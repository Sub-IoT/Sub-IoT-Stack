/*
 * systick.h
 *
 *  Created on: Mar 4, 2010
 *      Author: armin
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_
#include "stm32l1xx.h"

extern volatile uint32_t systickValue;

void systick_init();
void delaymS(uint32_t msec);
void delayuS(uint32_t uS);

void testUDelay();

#endif /* SYSTICK_H_ */
