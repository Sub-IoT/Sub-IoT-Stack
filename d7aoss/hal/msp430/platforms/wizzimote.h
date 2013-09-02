/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef WIZZIMOTE_H_
#define WIZZIMOTE_H_

#include "../driverlib/5xx_6xx/gpio.h"

#include "../../../types.h"
#include "../msp430_addresses.h"

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
