/*
 * msp430.h
 *
 *  Created on: 9-sep.-2013
 *      Author: Dragan Subotic
 */

#ifndef MSP430_H_
#define MSP430_H_

#include "../driverlib/5xx_6xx/gpio.h"

#include "../../../types.h"
#include "../msp430_addresses.h"

#define LED_RED	1


#define OUTPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT1_R__
#define OUTPUT1_PORT			GPIO_PORT_P1
#define OUTPUT1_PIN 			GPIO_PIN0

#define OUTPUT2_BASEADDRESS 	__MSP430_BASEADDRESS_PORT1_R__
#define OUTPUT2_PORT 			GPIO_PORT_P1
#define OUTPUT2_PIN 			GPIO_PIN0

#define OUTPUT3_BASEADDRESS  	__MSP430_BASEADDRESS_PORT1_R__
#define OUTPUT3_PORT 			GPIO_PORT_P1
#define OUTPUT3_PIN 			GPIO_PIN0

#define INPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT1_PORT				GPIO_PORT_P1
#define INPUT1_PIN				GPIO_PIN1

#define INPUT2_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT2_PORT				GPIO_PORT_P1
#define INPUT2_PIN				GPIO_PIN1

#define INPUT3_BASEADDRESS		__MSP430_BASEADDRESS_PORT2_R__
#define INPUT3_PORT				GPIO_PORT_P1
#define INPUT3_PIN				GPIO_PIN1



#endif /* MSP430_H_ */
