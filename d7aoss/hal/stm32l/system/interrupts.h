/*
 * interrupts.h
 *
 *  Created on: Jun 25, 2013
 *      Author: armin
 */

#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#define INTERRUPT_BUTTON1 	(1)
#define INTERRUPT_BUTTON2 	(1 << 1)
#define INTERRUPT_BUTTON3 	(1 << 2)
#define INTERRUPT_RTC 		(1 << 3)

extern volatile uint8_t interrupt_flags;


#endif /* INTERRUPTS_H_ */
