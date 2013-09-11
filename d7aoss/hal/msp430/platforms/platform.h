/*
 * platform.h
 *
 *  Created on: 4-dec.-2012
 *      Author: Maarten Weyn
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#define PLATFORM_F5172


#ifdef PLATFORM_F5172
#include "f5172.h"
#else
	#error No platform set
#endif


#endif /* PLATFORM_H_ */
