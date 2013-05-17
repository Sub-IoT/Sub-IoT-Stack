/*
 * platform.h
 *
 *  Created on: 4-dec.-2012
 *      Author: Maarten Weyn
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#define PLATFORM_WIZZIMOTE


#ifdef PLATFORM_WIZZIMOTE
#include "wizzimote.h"
#elif defined PLATFORM_ARTESIS
#include "artesis.h"
#elif defined PLATFORM_AGAIDI
#include "agaidi.h"
#else
	#error No platform set
#endif


#endif /* PLATFORM_H_ */
