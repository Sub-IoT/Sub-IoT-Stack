/*
 * platform.h
 *
 *  Created on: 4-dec.-2012
 *      Author: Maarten Weyn
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#define PLATFORM_MSP430


#ifdef PLATFORM_MSP430
#include "msp430.h"
#else
	#error No platform set
#endif


#endif /* PLATFORM_H_ */
