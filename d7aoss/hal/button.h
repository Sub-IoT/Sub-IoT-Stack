/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __BUTTON_H__
#define __BUTTON_H__



void button_init();

void button_enable_interrupts();

void button_disable_interrupts();

void button_clear_interrupt_flag();

unsigned char button_is_active(unsigned char);

#endif //__BUTTON_H__
