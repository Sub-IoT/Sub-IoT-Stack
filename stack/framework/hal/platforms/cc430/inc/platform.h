#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include "platform_defs.h"
#include "cc430_chip.h"

#ifndef PLATFORM_CC430
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_CC430 to be defined
#endif

/********************
 * LED DEFINITIONS *
 *******************/

#define HW_NUM_LEDS 3

//INT_HANDLER


/********************
 * UART DEFINITIONS *
 *******************/

#define UART_BAUDRATE PLATFORM_CC430_UART_BAUDRATE

// TODO

// UART0 location #1: PE0 and PE1
//#define UART_PIN_TX         E0           // PE0
//#define UART_PIN_RX         E1          // PE1

/*************************
 * DEBUG PIN DEFINITIONS *
 ************************/

//#define DEBUG_PIN_NUM 4
//#define DEBUG0	D4
//#define DEBUG1	D5
//#define DEBUG2	D6
//#define DEBUG3	D7

/**************************
 * USERBUTTON DEFINITIONS *
 *************************/

//#define NUM_USERBUTTONS 	2
//#define BUTTON0				B9
//#define BUTTON1				B10


#endif
