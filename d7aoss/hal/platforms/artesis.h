/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef ARTESIS_H_
#define ARTESIS_H_

#include "../driverlib/5xx_6xx/gpio.h"

#include "../types.h"
#include "../addresses.h"

#define OUTPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT1_R__
#define OUTPUT1_PORT			GPIO_PORT_P1
#define OUTPUT1_PIN 			GPIO_PIN1

#define OUTPUT2_BASEADDRESS 	__MSP430_BASEADDRESS_PORT1_R__
#define OUTPUT2_PORT 			GPIO_PORT_P1
#define OUTPUT2_PIN 			GPIO_PIN0

#define OUTPUT3_BASEADDRESS  	__MSP430_BASEADDRESS_PORT1_R__
#define OUTPUT3_PORT 			GPIO_PORT_P1
#define OUTPUT3_PIN 			GPIO_PIN1

#define INPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT1_R__
#define INPUT1_PORT				GPIO_PORT_P3
#define INPUT1_PIN				GPIO_PIN6

#define INPUT2_BASEADDRESS		__MSP430_BASEADDRESS_PORT1_R__
#define INPUT2_PORT				GPIO_PORT_P3
#define INPUT2_PIN				GPIO_PIN7

#define INPUT3_BASEADDRESS		__MSP430_BASEADDRESS_PORT1_R__
#define INPUT3_PORT				GPIO_PORT_P3
#define INPUT3_PIN				GPIO_PIN7

#endif /* ARTESIS_H_ */
