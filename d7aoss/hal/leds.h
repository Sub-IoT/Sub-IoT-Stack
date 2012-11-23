/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __LEDS_H__
#define __LEDS_H__

void led_init();

void led_on(unsigned char led_nr);
void led_off(unsigned char led_nr);
void led_toggle(unsigned char led_nr);


#endif // __LEDS_H__
