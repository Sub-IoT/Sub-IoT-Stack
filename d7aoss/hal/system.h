/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "../types.h"
#include "addresses.h"

// *************************************************************************************************
// Define section
#define st(x)      					do { x } while (__LINE__ == -1)
#define ENTER_CRITICAL_SECTION(x)  	__disable_interrupt(); //st( x = __read_status_register(); __disable_interrupt(); )
#define EXIT_CRITICAL_SECTION(x)    __enable_interrupt(); //__write_status_register(x)

extern u8 tag_id[8]; // TODO: keep this as global?

void System_Init();

void System_StopWatchdogTimer();
void System_StartWatchdogTimer();

void System_LowPowerMode(unsigned char mode, unsigned char enableInterrupts);

void System_GetUniqueId(unsigned char *tagId);

#endif // __SYSTEM_H__
