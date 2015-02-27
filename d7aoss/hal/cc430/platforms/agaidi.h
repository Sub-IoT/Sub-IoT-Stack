/*!
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
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *     	glenn.ergeerts@uantwerpen.be
 *
 */

#ifndef AGAIDI_H_
#define AGAIDI_H_

#include "../driverlib/5xx_6xx/gpio.h"

#include "../../../types.h"
#include "../cc430_addresses.h"

#define LED_RED	2
#define LED_ORANGE 1
#define LED_GREEN 3

// UART
// RX: P1.5
// TX: P1.6
#define PLATFORM_UCA0RXD	P1MAP5
#define PLATFORM_UCA0TXD	P1MAP6

#define PLATFORM_PxDIR 		P1DIR
#define PLATFORM_PxDIRBIT	BIT6
#define PLATFORM_PxSEL		P1SEL
#define PLATFORM_PxSELBIT	BIT5 + BIT6

// IO
#define OUTPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT3_R__
#define OUTPUT1_PORT			GPIO_PORT_P3
#define OUTPUT1_PIN 			GPIO_PIN4

#define OUTPUT2_BASEADDRESS 	__MSP430_BASEADDRESS_PORT3_R__
#define OUTPUT2_PORT 			GPIO_PORT_P3
#define OUTPUT2_PIN 			GPIO_PIN5

#define OUTPUT3_BASEADDRESS  	__MSP430_BASEADDRESS_PORT3_R__
#define OUTPUT3_PORT 			GPIO_PORT_P3
#define OUTPUT3_PIN 			GPIO_PIN3

#define OUTPUT1_TYPE			0
#define OUTPUT2_TYPE			0
#define OUTPUT3_TYPE			0

#define INPUT_INTERRUPT_VECTOR 	PORT2_VECTOR
#define INPUT_INTERRUPT_ISR		PORT2_ISR;

#define INPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT1_PORT				GPIO_PORT_P2
#define INPUT1_PIN				GPIO_PIN0

#define INPUT2_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT2_PORT				GPIO_PORT_P2
#define INPUT2_PIN				GPIO_PIN0

#define INPUT3_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT3_PORT				GPIO_PORT_P2
#define INPUT3_PIN				GPIO_PIN0

#define UART_RX					P1MAP5
#define UART_TX					P1MAP6

#endif /* AGAIDI_H_ */
