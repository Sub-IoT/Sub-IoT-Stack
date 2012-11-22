/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __LEDS_H__
#define __LEDS_H__

void Leds_Init();

void Led_On(unsigned char led_nr);
void Led_Off(unsigned char led_nr);
void Led_Toggle(unsigned char led_nr);


#endif // __LEDS_H__
