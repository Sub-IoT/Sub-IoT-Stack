/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2019 aloxy.io
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file hwrtc.h
 * \addtogroup timer
 * \ingroup HAL
 * @{
 * \brief RTC Timer API
 * \author maarten.weyn@uantwerpen.be
 *
 */

#ifndef HW_RTC_H_
#define HW_RTC_H_

#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include "link_c.h"
#include "hwtimer.h"


__LINK_C error_t hw_rtc_init( void );
__LINK_C error_t hw_rtc_setcallback(timer_callback_t cb);

__LINK_C const hwtimer_info_t* hw_rtc_get_info();

__LINK_C error_t hw_rtc_schedule_delay_ticks(uint32_t tick);
__LINK_C error_t hw_rtc_schedule_delay(uint8_t hours, uint8_t minutes, uint8_t seconds, uint32_t subseconds);

__LINK_C error_t hw_rtc_cancel();
__LINK_C error_t hw_rtc_reset();
__LINK_C error_t hw_rtc_set(uint16_t year, uint8_t month, uint8_t day, uint8_t hours, uint8_t minutes, uint8_t seconds, uint32_t subseconds);
__LINK_C bool hw_rtc_is_interrupt_pending();

__LINK_C uint32_t hw_rtc_getvalue();
__LINK_C error_t hw_rtc_getdatetime(RTC_TimeTypeDef* time, RTC_DateTypeDef* date);

#endif /* HW_RTC_H_ */

/** @}*/
