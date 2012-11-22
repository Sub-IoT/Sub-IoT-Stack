/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __RTC_H__
#define __RTC_H__

// Currrently 1 sec intervals
void Rtc_InitCounterMode();

void Rtc_EnableInterrupt();
void Rtc_DisableInterrupt();

void Rtc_Start();

void Rtc_Stop();

#endif // __RTC_H__
