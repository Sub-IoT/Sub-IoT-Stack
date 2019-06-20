/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2019 Aloxy.io
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

/*! \file stm32_common_rtctimer.c
 *
 */


#include "platform_defs.h"


#include <stdbool.h>
#include <stdint.h>

#include "hwtimer.h"
#include "hwatomic.h"
#include "stm32_device.h"
#include "debug.h"
#include "errors.h"
#include "log.h"


typedef uint32_t TimerTime_t;
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  TimerTime_t Rtc_Time; /* Reference time */
  
  RTC_TimeTypeDef RTC_Calndr_Time; /* Reference time in calendar format */

  RTC_DateTypeDef RTC_Calndr_Date; /* Reference date in calendar format */
  
} RtcTimerContext_t;


// TODO define timers in ports.h
#define HWTIMER_NUM 1

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_TIMER_LOG_ENABLED)
  #define DPRINT(...) log_print_stack_string(LOG_STACK_FWK, __VA_ARGS__)
#else
  #define DPRINT(...)
#endif

#define CONV_NUMER                (MSEC_NUMBER>>COMMON_FACTOR)
#define CONV_DENOM                (1<<(N_PREDIV_S-COMMON_FACTOR))

/* subsecond number of bits */
#define N_PREDIV_S                 10
/* Synchonuous prediv  */
#define PREDIV_S                  ((1<<N_PREDIV_S)-1)
/* Asynchonuous prediv   */
#define PREDIV_A                  (1<<(15-N_PREDIV_S))-1

/* Sub-second mask definition  */
#define HW_RTC_ALARMSUBSECONDMASK N_PREDIV_S<<RTC_ALRMASSR_MASKSS_Pos

#define  DAYS_IN_LEAP_YEAR (uint32_t) 366

#define  DAYS_IN_YEAR      (uint32_t) 365

#define  SECONDS_IN_1DAY   (uint32_t) 86400

#define  SECONDS_IN_1HOUR   (uint32_t) 3600

#define  SECONDS_IN_1MINUTE   (uint32_t) 60

#define  MINUTES_IN_1HOUR    (uint32_t) 60

#define  HOURS_IN_1DAY      (uint32_t) 24

#define  DAYS_IN_MONTH_CORRECTION_NORM     ((uint32_t) 0x99AAA0 )
#define  DAYS_IN_MONTH_CORRECTION_LEAP     ((uint32_t) 0x445550 )

/* Calculates ceiling(X/N) */
#define DIVC(X,N)   ( ( (X) + (N) -1 ) / (N) )

static timer_callback_t callback = 0x0;
static bool timer_inited = false;
static RTC_HandleTypeDef rtcHandle={0};
static RtcTimerContext_t rtcTimerContext;
static RTC_AlarmTypeDef rtcAlarmStructure;

/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


static TimerTime_t HW_RTC_GetCalendarValue( RTC_DateTypeDef* RTC_DateStruct, RTC_TimeTypeDef* RTC_TimeStruct )
{
  TimerTime_t calendarValue = 0;
  uint32_t first_read;
  uint32_t correction;
  
  /* Get Time and Date*/
  HAL_RTC_GetTime( &rtcHandle, RTC_TimeStruct, RTC_FORMAT_BIN );
 
   /* make sure it is correct due to asynchronus nature of RTC*/
  do {
    first_read = RTC_TimeStruct->SubSeconds;
    HAL_RTC_GetDate( &rtcHandle, RTC_DateStruct, RTC_FORMAT_BIN );
    HAL_RTC_GetTime( &rtcHandle, RTC_TimeStruct, RTC_FORMAT_BIN );
  } while (first_read != RTC_TimeStruct->SubSeconds);
 
  /* calculte amount of elapsed days since 01/01/2000 */
  calendarValue= DIVC( (DAYS_IN_YEAR*3 + DAYS_IN_LEAP_YEAR)* RTC_DateStruct->Year , 4);

  correction = ( (RTC_DateStruct->Year % 4) == 0 ) ? DAYS_IN_MONTH_CORRECTION_LEAP : DAYS_IN_MONTH_CORRECTION_NORM ;
 
  calendarValue +=( DIVC( (RTC_DateStruct->Month-1)*(30+31) ,2 ) - (((correction>> ((RTC_DateStruct->Month-1)*2) )&0x3)));

  calendarValue += (RTC_DateStruct->Date - 1);
  
  /* convert from days to seconds */
  calendarValue *= SECONDS_IN_1DAY; 

  calendarValue += ( ( uint32_t )RTC_TimeStruct->Seconds + 
                     ( ( uint32_t )RTC_TimeStruct->Minutes * SECONDS_IN_1MINUTE ) +
                     ( ( uint32_t )RTC_TimeStruct->Hours * SECONDS_IN_1HOUR ) ) ;

  calendarValue = (calendarValue<<N_PREDIV_S) + ( PREDIV_S - RTC_TimeStruct->SubSeconds);

  return( calendarValue );
}

static void rtc_setalarmconfig( void )
{
  HAL_RTC_DeactivateAlarm(&rtcHandle, RTC_ALARM_A);
}

uint32_t rtc_settimercontext( void )
{
  rtcTimerContext.Rtc_Time = HW_RTC_GetCalendarValue( &rtcTimerContext.RTC_Calndr_Date, &rtcTimerContext.RTC_Calndr_Time );
  return ( uint32_t ) rtcTimerContext.Rtc_Time;
}

// Sets up a timer to count at 1024 Hz, driven by the 32.768 kHz LSE
// The timer is running continuously. On STM32L0 the RTC is used
error_t hw_rtc_init()
{
  TIM_ClockConfigTypeDef clock_source_config;
  TIM_MasterConfigTypeDef master_config;

  start_atomic();

  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;

  rtcHandle.Instance = RTC;

  rtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  rtcHandle.Init.AsynchPrediv = PREDIV_A; /* RTC_ASYNCH_PREDIV; */
  rtcHandle.Init.SynchPrediv = PREDIV_S; /* RTC_SYNCH_PREDIV; */
  rtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  rtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  rtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

  HAL_RTC_Init( &rtcHandle );
  
  /*Monday 1st January 2019*/
  RTC_DateStruct.Year = 0;
  RTC_DateStruct.Month = RTC_MONTH_JANUARY;
  RTC_DateStruct.Date = 1;
  RTC_DateStruct.WeekDay = RTC_WEEKDAY_MONDAY;
  HAL_RTC_SetDate(&rtcHandle , &RTC_DateStruct, RTC_FORMAT_BIN);

  /*at 0:0:0*/
  RTC_TimeStruct.Hours = 0;
  RTC_TimeStruct.Minutes = 0;

  RTC_TimeStruct.Seconds = 0;
  RTC_TimeStruct.TimeFormat = 0;
  RTC_TimeStruct.SubSeconds = 0;
  RTC_TimeStruct.StoreOperation = RTC_DAYLIGHTSAVING_NONE;
  RTC_TimeStruct.DayLightSaving = RTC_STOREOPERATION_RESET;

  HAL_RTC_SetTime(&rtcHandle , &RTC_TimeStruct, RTC_FORMAT_BIN);

 /*Enable Direct Read of the calendar registers (not through Shadow) */
  HAL_RTCEx_EnableBypassShadow(&rtcHandle);

	timer_inited = true;
  end_atomic();

  rtc_setalarmconfig();
  rtc_settimercontext();

  return SUCCESS;
}

error_t hw_rtc_deinit() {
  if (rtcHandle.Instance  == RTC) {
    HAL_RTC_Init( &rtcHandle );
    return SUCCESS;
  }

  return ERROR;
}



__LINK_C error_t hw_rtc_setcallback(timer_callback_t cb)
{
  callback = cb;
  return SUCCESS;
}

const hwtimer_info_t* hw_rtc_get_info()
{
    static const hwtimer_info_t timer_info = {
      .min_delay_ticks = 3, 
    };

    return &timer_info;
}

uint32_t hw_rtc_getvalue()
{
 	if(!timer_inited) {
 		return 0;
  } else {
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
    
    uint32_t calendarValue = (uint32_t) HW_RTC_GetCalendarValue(&RTC_DateStruct, &RTC_TimeStruct );

    return calendarValue;
 	}
}

error_t hw_rtc_getdatetime(RTC_TimeTypeDef* time, RTC_DateTypeDef* date)
{
 	if(!timer_inited) {
 		return ERROR;
  } else {
    
    uint32_t calendarValue = (uint32_t) HW_RTC_GetCalendarValue(date, time);

    return SUCCESS;
 	}
}

error_t hw_rtc_schedule_delay_ticks(uint32_t tick)
{
  DPRINT("hw_rtc_schedule_delay %d\n", tick);

  start_atomic();
  uint16_t rtcAlarmSubSeconds = 0;
  uint16_t rtcAlarmSeconds = 0;
  uint16_t rtcAlarmMinutes = 0;
  uint16_t rtcAlarmHours = 0;
  uint16_t rtcAlarmDays = 0;
  RTC_TimeTypeDef RTC_TimeStruct = rtcTimerContext.RTC_Calndr_Time;
  RTC_DateTypeDef RTC_DateStruct = rtcTimerContext.RTC_Calndr_Date;


  /* Clear RTC Alarm Flag */
  __HAL_RTC_ALARM_CLEAR_FLAG( &rtcHandle, RTC_FLAG_ALRAF);
  /* Disable the Alarm A interrupt */
  HAL_RTC_DeactivateAlarm(&rtcHandle, RTC_ALARM_A);

  /*reverse counter */
  rtcAlarmSubSeconds = ( tick & PREDIV_S);
  /* convert timeout  to seconds */
  tick >>= N_PREDIV_S;  /* convert timeout  in seconds */

  /*convert microsecs to RTC format and add to 'Now' */
  rtcAlarmDays =  1;

  while (tick >= SECONDS_IN_1DAY)
  {
    tick -= SECONDS_IN_1DAY;
    rtcAlarmDays++;
  }

  /* calc hours */
  rtcAlarmHours = 0;
  while (tick >= SECONDS_IN_1HOUR)
  {
    tick -= SECONDS_IN_1HOUR;
    rtcAlarmHours++;
  }

  /* calc minutes */
  rtcAlarmMinutes = 0;
  while (tick >= SECONDS_IN_1MINUTE)
  {
    tick -= SECONDS_IN_1MINUTE;
    rtcAlarmMinutes++;
  }

  /* calc seconds */
  rtcAlarmSeconds =  tick;

  /***** correct for modulo********/
  while (rtcAlarmSubSeconds >= (PREDIV_S+1))
  {
    rtcAlarmSubSeconds -= (PREDIV_S+1);
    rtcAlarmSeconds++;
  }

  while (rtcAlarmSeconds >= SECONDS_IN_1MINUTE)
  { 
    rtcAlarmSeconds -= SECONDS_IN_1MINUTE;
    rtcAlarmMinutes++;
  }

  while (rtcAlarmMinutes >= MINUTES_IN_1HOUR)
  {
    rtcAlarmMinutes -= MINUTES_IN_1HOUR;
    rtcAlarmHours++;
  }

  while (rtcAlarmHours >= HOURS_IN_1DAY)
  {
    rtcAlarmHours -= HOURS_IN_1DAY;
    rtcAlarmDays++;
  }

  if( RTC_DateStruct.Year % 4 == 0 ) 
  {
    if( rtcAlarmDays > DaysInMonthLeapYear[ RTC_DateStruct.Month - 1 ] )    
    {
      rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[ RTC_DateStruct.Month - 1 ];
    }
  }
  else
  {
    if( rtcAlarmDays > DaysInMonth[ RTC_DateStruct.Month - 1 ] )    
    {   
      rtcAlarmDays = rtcAlarmDays % DaysInMonth[ RTC_DateStruct.Month - 1 ];
    }
  }

  /* Set rtcAlarmStructure with calculated values*/
  rtcAlarmStructure.AlarmTime.SubSeconds = PREDIV_S-rtcAlarmSubSeconds;
  rtcAlarmStructure.AlarmSubSecondMask  = HW_RTC_ALARMSUBSECONDMASK; 
  rtcAlarmStructure.AlarmTime.Seconds = rtcAlarmSeconds;
  rtcAlarmStructure.AlarmTime.Minutes = rtcAlarmMinutes;
  rtcAlarmStructure.AlarmTime.Hours   = rtcAlarmHours;
  rtcAlarmStructure.AlarmDateWeekDay    = ( uint8_t )rtcAlarmDays;
  rtcAlarmStructure.AlarmTime.TimeFormat   = RTC_TimeStruct.TimeFormat;
  rtcAlarmStructure.AlarmDateWeekDaySel   = RTC_ALARMDATEWEEKDAYSEL_DATE; 
  rtcAlarmStructure.AlarmMask       = RTC_ALARMMASK_NONE;
  rtcAlarmStructure.Alarm = RTC_ALARM_A;
  rtcAlarmStructure.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  rtcAlarmStructure.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;

  DPRINT("HAL_RTC_SetAlarm_IT %d %d:%d:%d.%d\n", 
    rtcAlarmStructure.AlarmDateWeekDay,
    rtcAlarmStructure.AlarmTime.Hours,
    rtcAlarmStructure.AlarmTime.Minutes,
    rtcAlarmStructure.AlarmTime.Seconds,
    rtcAlarmStructure.AlarmTime.SubSeconds);

  /* Set RTC_Alarm */
  HAL_StatusTypeDef ret = HAL_RTC_SetAlarm_IT( &rtcHandle, &rtcAlarmStructure, RTC_FORMAT_BIN );
  if (ret != HAL_OK) DPRINT("HAL_RTC_SetAlarm_IT error %d\n", ret);

  end_atomic();

  /// debug
#ifdef DEBUG
  uint32_t first_read;
  /* Get Time and Date*/
  HAL_RTC_GetTime( &rtcHandle, &RTC_TimeStruct, RTC_FORMAT_BIN );
 
   /* make sure it is correct due to asynchronus nature of RTC*/
  do {
    first_read = RTC_TimeStruct.SubSeconds;
    HAL_RTC_GetDate( &rtcHandle, &RTC_DateStruct, RTC_FORMAT_BIN );
    HAL_RTC_GetTime( &rtcHandle, &RTC_TimeStruct, RTC_FORMAT_BIN );
  } while (first_read != RTC_TimeStruct.SubSeconds);

  DPRINT("Current time %d-%d-%d %d:%d:%d.%d\n", 
  RTC_DateStruct.Year,
  RTC_DateStruct.Month,
  RTC_DateStruct.Date,
  RTC_TimeStruct.Hours,
  RTC_TimeStruct.Minutes,
  RTC_TimeStruct.Seconds,
  RTC_TimeStruct.SubSeconds);
#endif

  return SUCCESS;
}

error_t hw_rtc_schedule_delay(uint8_t hours, uint8_t minutes, uint8_t seconds, uint32_t subseconds)
{
  
  DPRINT("hw_rtc_schedule_delay %d:%d:%d.%d\n", hours, minutes, seconds, subseconds);

  uint32_t first_read;
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;
  
  /* Get Time and Date*/
  HAL_RTC_GetTime( &rtcHandle, &time, RTC_FORMAT_BIN );
 
   /* make sure it is correct due to asynchronus nature of RTC*/
  do {
    first_read = time.SubSeconds;
    HAL_RTC_GetDate( &rtcHandle, &date, RTC_FORMAT_BIN );
    HAL_RTC_GetTime( &rtcHandle, &time, RTC_FORMAT_BIN );
  } while (first_read != time.SubSeconds);

  
  time.SubSeconds += subseconds;
  time.Seconds += seconds;
  time.Minutes += minutes;
  time.Hours += hours;


  start_atomic();
  /* Clear RTC Alarm Flag */
  __HAL_RTC_ALARM_CLEAR_FLAG( &rtcHandle, RTC_FLAG_ALRAF);
  /* Disable the Alarm A interrupt */
  HAL_RTC_DeactivateAlarm(&rtcHandle, RTC_ALARM_A);

  if (time.SubSeconds >= PREDIV_S + 1)
  {
    time.SubSeconds -= (PREDIV_S+1);
    time.Seconds++;
  }

  if (time.Seconds >= SECONDS_IN_1MINUTE)
  {
    time.Seconds -= SECONDS_IN_1MINUTE;
    time.Minutes++;
  }

  if (time.Minutes >= MINUTES_IN_1HOUR)
  {
    time.Minutes -= MINUTES_IN_1HOUR;
    time.Hours++;
  }

  if (time.Hours >= HOURS_IN_1DAY)
  {
    time.Hours -= HOURS_IN_1DAY;
    date.Date++;
  }

  if( date.Year % 4 == 0 ) 
  {
    if( date.Date > DaysInMonthLeapYear[ date.Month - 1 ] )    
    {
      date.Date = date.Date % DaysInMonthLeapYear[ date.Month - 1 ];
    }
  }
  else
  {
    if( date.Date > DaysInMonth[ date.Month - 1 ] )    
    {   
      date.Date = date.Date % DaysInMonth[ date.Month - 1 ];
    }
  }

  /* Set rtcAlarmStructure with calculated values*/
  rtcAlarmStructure.AlarmTime.SubSeconds = PREDIV_S-time.SubSeconds;
  rtcAlarmStructure.AlarmSubSecondMask  = HW_RTC_ALARMSUBSECONDMASK; 
  rtcAlarmStructure.AlarmTime.Seconds = time.Seconds;
  rtcAlarmStructure.AlarmTime.Minutes = time.Minutes;
  rtcAlarmStructure.AlarmTime.Hours   = time.Hours;
  rtcAlarmStructure.AlarmDateWeekDay    = date.Date;
  rtcAlarmStructure.AlarmTime.TimeFormat   = time.TimeFormat;
  rtcAlarmStructure.AlarmDateWeekDaySel   = RTC_ALARMDATEWEEKDAYSEL_DATE; 
  rtcAlarmStructure.AlarmMask       = RTC_ALARMMASK_NONE;
  rtcAlarmStructure.Alarm = RTC_ALARM_A;
  rtcAlarmStructure.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  rtcAlarmStructure.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;

  DPRINT("HAL_RTC_SetAlarm_IT %d %d:%d:%d.%d\n", 
    rtcAlarmStructure.AlarmDateWeekDay,
    rtcAlarmStructure.AlarmTime.Hours,
    rtcAlarmStructure.AlarmTime.Minutes,
    rtcAlarmStructure.AlarmTime.Seconds,
    rtcAlarmStructure.AlarmTime.SubSeconds);

  /* Set RTC_Alarm */
  HAL_StatusTypeDef ret = HAL_RTC_SetAlarm_IT( &rtcHandle, &rtcAlarmStructure, RTC_FORMAT_BIN );
  if (ret != HAL_OK) DPRINT("HAL_RTC_SetAlarm_IT error %d\n", ret);

  end_atomic();

  /// debug
#ifdef DEBUG
  uint32_t first_read;
  /* Get Time and Date*/
  HAL_RTC_GetTime( &rtcHandle, &RTC_TimeStruct, RTC_FORMAT_BIN );
 
   /* make sure it is correct due to asynchronus nature of RTC*/
  do {
    first_read = RTC_TimeStruct.SubSeconds;
    HAL_RTC_GetDate( &rtcHandle, &RTC_DateStruct, RTC_FORMAT_BIN );
    HAL_RTC_GetTime( &rtcHandle, &RTC_TimeStruct, RTC_FORMAT_BIN );
  } while (first_read != RTC_TimeStruct.SubSeconds);

  DPRINT("Current time %d-%d-%d %d:%d:%d.%d\n", 
  RTC_DateStruct.Year,
  RTC_DateStruct.Month,
  RTC_DateStruct.Date,
  RTC_TimeStruct.Hours,
  RTC_TimeStruct.Minutes,
  RTC_TimeStruct.Seconds,
  RTC_TimeStruct.SubSeconds);
#endif

  return SUCCESS;
}

error_t hw_rtc_cancel()
{
 	start_atomic();
  /* Disable the Alarm A interrupt */
  HAL_RTC_DeactivateAlarm(&rtcHandle, RTC_ALARM_A );

  /* Clear RTC Alarm Flag */
  __HAL_RTC_ALARM_CLEAR_FLAG( &rtcHandle, RTC_FLAG_ALRAF);

  /* Clear the EXTI's line Flag for RTC Alarm */
  __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();
 	end_atomic();

  return SUCCESS;
}

error_t hw_rtc_set(uint16_t year, uint8_t month, uint8_t day, uint8_t hours, uint8_t minutes, uint8_t seconds, uint32_t subseconds)
{
  hw_rtc_cancel();

  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;

 	start_atomic();
  /*Monday 1st January 2019*/
  RTC_DateStruct.Year = year;
  RTC_DateStruct.Month = month;
  RTC_DateStruct.Date = day;
  //RTC_DateStruct.WeekDay = RTC_WEEKDAY_MONDAY;
  HAL_RTC_SetDate(&rtcHandle , &RTC_DateStruct, RTC_FORMAT_BIN);

  /*at 0:0:0*/
  RTC_TimeStruct.Hours = hours;
  RTC_TimeStruct.Minutes = minutes;

  RTC_TimeStruct.Seconds = seconds;
  RTC_TimeStruct.TimeFormat = 0;
  RTC_TimeStruct.SubSeconds = subseconds;
  RTC_TimeStruct.StoreOperation = RTC_DAYLIGHTSAVING_NONE;
  RTC_TimeStruct.DayLightSaving = RTC_STOREOPERATION_RESET;

  HAL_RTC_SetTime(&rtcHandle , &RTC_TimeStruct, RTC_FORMAT_BIN);

 	end_atomic();

  return SUCCESS;
}

error_t hw_rtc_reset()
{
  hw_rtc_set(0, RTC_MONTH_JANUARY, 1, 0, 0, 0, 0);
}



bool hw_rtc_is_interrupt_pending()
{
  start_atomic();
  bool is_pending = __HAL_RTC_ALARM_GET_FLAG(&rtcHandle, RTC_FLAG_ALRAF);
  end_atomic();

  return is_pending;
}



void RTC_IRQHandler(void)
{
  RTC_HandleTypeDef* hrtc=&rtcHandle;
  /* enable low power at irq*/
  //LowPower_Enable( e_LOW_POWER_RTC );
  
    /* Get the AlarmA interrupt source enable status */
  if(__HAL_RTC_ALARM_GET_IT_SOURCE(hrtc, RTC_IT_ALRA) != RESET)
  {
    /* Get the pending status of the AlarmA Interrupt */
    if(__HAL_RTC_ALARM_GET_FLAG(hrtc, RTC_FLAG_ALRAF) != RESET)
    {
      /* Clear the AlarmA interrupt pending bit */
      __HAL_RTC_ALARM_CLEAR_FLAG(hrtc, RTC_FLAG_ALRAF); 
      /* Clear the EXTI's line Flag for RTC Alarm */
      __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();
      /* AlarmA callback */
      if (callback != 0x0) callback();
    }
  }

  //todo: use only time, not date and use overflow_f when day changes.
}
