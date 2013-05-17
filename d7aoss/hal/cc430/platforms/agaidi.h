/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef AGAIDI_H_
#define AGAIDI_H_

#include "../driverlib/5xx_6xx/gpio.h"

#include "../../types.h"
#include "../addresses.h"

#define LED_RED	2
#define LED_ORANGE 1
#define LED_GREEN 3

#define OUTPUT1_BASEADDRESS		__MSP430_BASEADDRESS_PORT3_R__
#define OUTPUT1_PORT			GPIO_PORT_P3
#define OUTPUT1_PIN 			GPIO_PIN4

#define OUTPUT2_BASEADDRESS 	__MSP430_BASEADDRESS_PORT3_R__
#define OUTPUT2_PORT 			GPIO_PORT_P3
#define OUTPUT2_PIN 			GPIO_PIN5

#define OUTPUT3_BASEADDRESS  	__MSP430_BASEADDRESS_PORT3_R__
#define OUTPUT3_PORT 			GPIO_PORT_P3
#define OUTPUT3_PIN 			GPIO_PIN3

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
