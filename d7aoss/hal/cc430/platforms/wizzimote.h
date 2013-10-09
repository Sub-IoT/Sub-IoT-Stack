/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *     	glenn.ergeerts@uantwerpen.be
 *
 */

#ifndef WIZZIMOTE_H_
#define WIZZIMOTE_H_

#include "../driverlib/5xx_6xx/gpio.h"

#include "../../../types.h"
#include "../cc430_addresses.h"

#define LED_RED	1
#define LED_GREEN 2
#define LED_ORANGE 3

#define OUTPUT1_BASEADDRESS	__MSP430_BASEADDRESS_PORT1_R__
#define OUTPUT1_PORT			GPIO_PORT_P1
#define OUTPUT1_PIN 			GPIO_PIN7

#define OUTPUT2_BASEADDRESS 	__MSP430_BASEADDRESS_PORT3_R__
#define OUTPUT2_PORT 			GPIO_PORT_P3
#define OUTPUT2_PIN 			GPIO_PIN7

#define OUTPUT3_BASEADDRESS  	__MSP430_BASEADDRESS_PORT3_R__
#define OUTPUT3_PORT 			GPIO_PORT_P3
#define OUTPUT3_PIN 			GPIO_PIN6

#define INPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT1_R__
#define INPUT1_PORT				GPIO_PORT_P1
#define INPUT1_PIN				GPIO_PIN6

#define INPUT2_BASEADDRESS		__MSP430_BASEADDRESS_PORT1_R__
#define INPUT2_PORT				GPIO_PORT_P1
#define INPUT2_PIN				GPIO_PIN5

#define INPUT3_BASEADDRESS		__MSP430_BASEADDRESS_PORT1_R__
#define INPUT3_PORT				GPIO_PORT_P1
#define INPUT3_PIN				GPIO_PIN4

#endif /* WIZZIMOTE_H_ */
