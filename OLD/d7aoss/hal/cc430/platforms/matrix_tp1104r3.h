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
 *
 */

#ifndef MATRIX_TP1104R3_H_
#define MATRIX_TP1104R3_H_

#include "../driverlib/5xx_6xx/gpio.h"

#include "../../../types.h"
#include "../cc430_addresses.h"

#define LED_GREEN	1
#define LED_BLUE	2
#define LED_RED		3

// UART
#define PLATFORM_UCA0RXD	P2MAP6
#define PLATFORM_UCA0TXD	P2MAP4

#define PLATFORM_PxDIR 		P2DIR
#define PLATFORM_PxDIRBIT	BIT4
#define PLATFORM_PxSEL		P2SEL
#define PLATFORM_PxSELBIT	BIT4 + BIT6

// IO
#define OUTPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define OUTPUT1_PORT			GPIO_PORT_P2
#define OUTPUT1_PIN 			GPIO_PIN1
#define OUTPUT1_TYPE			1

#define OUTPUT2_BASEADDRESS 	__MSP430_BASEADDRESS_PORT2_R__
#define OUTPUT2_PORT 			GPIO_PORT_P2
#define OUTPUT2_PIN 			GPIO_PIN2
#define OUTPUT2_TYPE			1

#define OUTPUT3_BASEADDRESS  	__MSP430_BASEADDRESS_PORT2_R__
#define OUTPUT3_PORT 			GPIO_PORT_P2
#define OUTPUT3_PIN 			GPIO_PIN3
#define OUTPUT3_TYPE			1


// No Input
#define INPUT1_BASEADDRESS		NULL
#define INPUT1_PORT				NULL
#define INPUT1_PIN				NULL

#define INPUT2_BASEADDRESS		NULL
#define INPUT2_PORT				NULL
#define INPUT2_PIN				NULL

#define INPUT3_BASEADDRESS		NULL
#define INPUT3_PORT				NULL
#define INPUT3_PIN				NULL

#endif /* MATRIX_TP1104R3_H_ */
