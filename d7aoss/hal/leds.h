/*! \file leds.h
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * \author maarten.weyn@uantwerpen.be
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */

#ifndef __LEDS_H__
#define __LEDS_H__


#define RED_BASEADDRESS        __MSP430_BASEADDRESS_PORT2_R__
#define RED_PORT            GPIO_PORT_P2
#define RED_PIN             GPIO_PIN3
#define RED_TYPE            0

#define GREEN_BASEADDRESS     __MSP430_BASEADDRESS_PORT2_R__
#define GREEN_PORT             GPIO_PORT_P2
#define GREEN_PIN             GPIO_PIN4
#define GREEN_TYPE            0

#define BLUE_BASEADDRESS      __MSP430_BASEADDRESS_PORT2_R__
#define BLUE_PORT             GPIO_PORT_P2
#define BLUE_PIN             GPIO_PIN5
#define BLUE_TYPE            0

void led_init();

void led_on(unsigned char led_nr);
void led_off(unsigned char led_nr);
void led_toggle(unsigned char led_nr);


#endif // __LEDS_H__
