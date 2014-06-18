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
 *     	dragan.subotic@uantwerpen.be
 *
 */

#ifndef BELIGUM_H_
#define BELIGUM_H_

#include "../driverlib/5xx_6xx/gpio.h"

#include "../cc430_addresses.h"

#define LED_RED	0
#define LED_GREEN 0

// UART
#define PLATFORM_UCA0RXD	P1MAP5
#define PLATFORM_UCA0TXD	P1MAP6

#define PLATFORM_PxDIR 		P1DIR
#define PLATFORM_PxDIRBIT	BIT6
#define PLATFORM_PxSEL		P1SEL
#define PLATFORM_PxSELBIT	BIT5 + BIT6

// IO

#define OUTPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT3_R__
#define OUTPUT1_PORT			GPIO_PORT_P3
#define OUTPUT1_PIN 			GPIO_PIN0

#define OUTPUT2_BASEADDRESS 	__MSP430_BASEADDRESS_PORT3_R__
#define OUTPUT2_PORT 			GPIO_PORT_P3
#define OUTPUT2_PIN 			GPIO_PIN1

#define OUTPUT3_BASEADDRESS  	__MSP430_BASEADDRESS_PORT3_R__
#define OUTPUT3_PORT 			GPIO_PORT_P3
#define OUTPUT3_PIN 			GPIO_PIN0

#define OUTPUT1_TYPE			0
#define OUTPUT2_TYPE			0
#define OUTPUT3_TYPE			0

#define INPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT1_PORT				GPIO_PORT_P1
#define INPUT1_PIN				GPIO_PIN0

#define INPUT2_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT2_PORT				GPIO_PORT_P1
#define INPUT2_PIN				GPIO_PIN1

#define INPUT3_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT3_PORT				GPIO_PORT_P1
#define INPUT3_PIN				GPIO_PIN1

#endif /* BELIGUM_H_ */
