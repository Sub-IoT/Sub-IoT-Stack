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
