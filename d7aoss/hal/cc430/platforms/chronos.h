/*!
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
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *
 */

#ifndef CHRONOS_H_
#define CHRONOS_H_

#include "../driverlib/5xx_6xx/gpio.h"

#include "../cc430_addresses.h"

#define OUTPUT1_BASEADDRESS		0x0A22 // 0x0A22
#define OUTPUT1_PORT			0x00
#define OUTPUT1_PIN 			GPIO_PIN3

#define OUTPUT2_BASEADDRESS 	0x0A21 // 0x0A21
#define OUTPUT2_PORT 			0x00
#define OUTPUT2_PIN 			GPIO_PIN3

#define OUTPUT3_BASEADDRESS  	0x0A20 // 0x0A20
#define OUTPUT3_PORT 			0x00
#define OUTPUT3_PIN 			GPIO_PIN3

#define OUTPUT1_TYPE			0
#define OUTPUT2_TYPE			0
#define OUTPUT3_TYPE			0

#define INPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT1_PORT				GPIO_PORT_P2
#define INPUT1_PIN				GPIO_PIN2

#define INPUT2_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT2_PORT				GPIO_PORT_P2
#define INPUT2_PIN				GPIO_PIN1

#define INPUT3_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT3_PORT				GPIO_PORT_P2
#define INPUT3_PIN				GPIO_PIN4

#define INPUT4_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT4_PORT				GPIO_PORT_P2
#define INPUT4_PIN				GPIO_PIN0

#define INPUT5_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT5_PORT				GPIO_PORT_P2
#define INPUT5_PIN				GPIO_PIN3

#endif /* CHRONOS_H_ */
