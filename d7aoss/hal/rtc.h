/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __RTC_H__
#define __RTC_H__

// Currrently 1 sec intervals
void rtc_init_counter_mode();

void rtc_enable_interrupt();
void rtc_disable_interrupt();

void rtc_start();
void rtc_stop();

#endif // __RTC_H__
