/*
 * platform.h
 *
 *  Created on: 4-dec.-2012
 *      Author: Maarten Weyn
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

<<<<<<< Updated upstream
#define PLATFORM_F5172


#ifdef PLATFORM_F5172
#include "f5172.h"
=======
#define PLATFORM_MSP430


#ifdef PLATFORM_WIZZIMOTE
#include "wizzimote.h"
#elif defined PLATFORM_ARTESIS
#include "artesis.h"
#elif defined PLATFORM_AGAIDI
#include "agaidi.h"
#elif defined PLATFORM_MSP430
#include "msp430.h"
>>>>>>> Stashed changes
#else
	#error No platform set
#endif


#endif /* PLATFORM_H_ */
