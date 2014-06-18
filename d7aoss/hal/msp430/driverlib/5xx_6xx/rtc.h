/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
#ifndef __MSP430WARE_RTC_H__
#define __MSP430WARE_RTC_H__

//*****************************************************************************
//
//The following are the defines to include the required modules for this
//peripheral in msp430xgeneric.h file
//
//*****************************************************************************
#define __MSP430_HAS_RTC__
#define __MSP430_HAS_RTC_B__

//*****************************************************************************
//
//The following is a struct that can be passed to RTC_CalendarInit() in the
//CalendarTime parameter, as well as returned by RTC_getCalendarTime()
//
//*****************************************************************************
typedef struct {
    unsigned char Seconds;
    unsigned char Minutes;
    unsigned char Hours;
    unsigned char DayOfWeek;
    unsigned char DayOfMonth;
    unsigned char Month;
    unsigned int Year;
} Calendar;

//*****************************************************************************
//
//The following are values that can be passed to RTC_setCalibrationData()
//in the offsetDirection parameter.
//
//*****************************************************************************
#define RTC_CALIBRATIONFREQ_OFF   (RTCCALF_0)
#define RTC_CALIBRATIONFREQ_512HZ (RTCCALF_1)
#define RTC_CALIBRATIONFREQ_256HZ (RTCCALF_2)
#define RTC_CALIBRATIONFREQ_1HZ   (RTCCALF_3)

//*****************************************************************************
//
//The following are values that can be passed to RTC_setCalibrationData()
//in the offsetDirection parameter.
//
//*****************************************************************************
#define RTC_CALIBRATION_DOWN2PPM  ( !(RTCCALS) )
#define RTC_CALIBRATION_UP4PPM    (RTCCALS)

//*****************************************************************************
//
//The following are values that can be passed to RTC_setClockRegistersFormat()
//in the formatSelect parameter.
//
//*****************************************************************************
#define RTC_FORMAT_BINARY  ( !(RTCBCD) )
#define RTC_FORMAT_BCD     (RTCBCD)

//*****************************************************************************
//
//The following are values that can be passed to RTC_counterInit()
//in the clockSelect parameter.
//
//*****************************************************************************
#define RTC_CLOCKSELECT_ACLK  (RTCSSEL_0)
#define RTC_CLOCKSELECT_SMCLK (RTCSSEL_1)
#define RTC_CLOCKSELECT_RT1PS (RTCSSEL_2)

//*****************************************************************************
//
//The following are values that can be passed to RTC_counterInit()
//in the counterSizeSelect parameter.
//
//*****************************************************************************
#define RTC_COUNTERSIZE_8BIT  (RTCTEV_0)
#define RTC_COUNTERSIZE_16BIT (RTCTEV_1)
#define RTC_COUNTERSIZE_24BIT (RTCTEV_2)
#define RTC_COUNTERSIZE_32BIT (RTCTEV_3)

//*****************************************************************************
//
//The following is a value that can be passed to RTC_setCalendarAlarm() in the
//minutesAlarm, hoursAlarm, dayOfWeekAlarm, and dayOfMonthAlarm parameters.
//
//*****************************************************************************
#define RTC_ALARMCONDITION_OFF  (0x80)

//*****************************************************************************
//
//The following are values that can be passed to RTC_setCalendarEvent()
//in the eventSelect parameter.
//
//*****************************************************************************
#define RTC_CALENDAREVENT_MINUTECHANGE  (RTCTEV_0)
#define RTC_CALENDAREVENT_HOURCHANGE    (RTCTEV_1)
#define RTC_CALENDAREVENT_NOON          (RTCTEV_2)
#define RTC_CALENDAREVENT_MIDNIGHT      (RTCTEV_3)

//*****************************************************************************
//
//The following are values that can be passed to RTC_counterPrescaleInit(),
//RTC_definePreScaleEvent(), RTC_readPrescaleCounterValue(), and
//RTC_setPrescaleCounterValue() in the prescaleSelect parameter.
//
//*****************************************************************************
#define RTC_PRESCALE_0  (0x0)
#define RTC_PRESCALE_1  (0x2)

//*****************************************************************************
//
//The following are values that can be passed to RTC_counterPrescaleInit()
//in the prescaleClockSelect parameter.
//
//*****************************************************************************
#define RTC_PSCLOCKSELECT_ACLK  (RT1SSEL_0)
#define RTC_PSCLOCKSELECT_SMCLK (RT1SSEL_1)
#define RTC_PSCLOCKSELECT_RT0PS (RT1SSEL_2)

//*****************************************************************************
//
//The following are values that can be passed to RTC_counterPrescaleInit()
//in the prescaleDivider parameter.
//
//*****************************************************************************
#define RTC_PSDIVIDER_2   (RT0PSDIV_0)
#define RTC_PSDIVIDER_4   (RT0PSDIV_1)
#define RTC_PSDIVIDER_8   (RT0PSDIV_2)
#define RTC_PSDIVIDER_16  (RT0PSDIV_3)
#define RTC_PSDIVIDER_32  (RT0PSDIV_4)
#define RTC_PSDIVIDER_64  (RT0PSDIV_5)
#define RTC_PSDIVIDER_128 (RT0PSDIV_6)
#define RTC_PSDIVIDER_256 (RT0PSDIV_7)

//*****************************************************************************
//
//The following are values that can be passed to RTC_definePrescaleEvent()
//in the prescaleEventDivider parameter.
//
//*****************************************************************************
#define RTC_PSEVENTDIVIDER_2   (RT0IP_0)
#define RTC_PSEVENTDIVIDER_4   (RT0IP_1)
#define RTC_PSEVENTDIVIDER_8   (RT0IP_2)
#define RTC_PSEVENTDIVIDER_16  (RT0IP_3)
#define RTC_PSEVENTDIVIDER_32  (RT0IP_4)
#define RTC_PSEVENTDIVIDER_64  (RT0IP_5)
#define RTC_PSEVENTDIVIDER_128 (RT0IP_6)
#define RTC_PSEVENTDIVIDER_256 (RT0IP_7)

//*****************************************************************************
//
//The following are values that can be passed to RTC_getInterruptStatus(),
//RTC_clearInterrupt(), RTC_enableInterrupt(),  RTC_disableInterrupt()
//in the interruptFlagMask parameter.
//
//*****************************************************************************
#define RTC_OSCILLATOR_FAULT_INTERRUPT  RTCOFIE   
#define RTC_TIME_EVENT_INTERRUPT        RTCTEVIE  
#define RTC_CLOCK_ALARM_INTERRUPT       RTCAIE    
#define RTC_CLOCK_READ_READY_INTERRUPT  RTCRDYIE  
#define RTC_PRESCALE_TIMER0_INTERRUPT   0x02
#define RTC_PRESCALE_TIMER1_INTERRUPT   0x01


//*****************************************************************************
//
//Prototypes for the APIs.
//
//*****************************************************************************
extern void RTC_startClock (unsigned int baseAddress);

extern void RTC_holdClock (unsigned int baseAddress);

extern void RTC_setCalibrationFrequency (unsigned int baseAddress,
    unsigned int frequencySelect);

extern void RTC_setCalibrationData (unsigned int baseAddress,
    unsigned char offsetDirection,
    unsigned char offsetValue);

extern void RTC_counterInit (unsigned int baseAddress,
    unsigned int clockSelect,
    unsigned int counterSizeSelect);

extern void RTC_calendarInit (unsigned int baseAddress,
    Calendar CalendarTime,
    unsigned int formatSelect);

extern Calendar RTC_getCalendarTime (unsigned int baseAddress);

extern void RTC_setCalendarAlarm (unsigned int baseAddress,
    unsigned char minutesAlarm,
    unsigned char hoursAlarm,
    unsigned char dayOfWeekAlarm,
    unsigned char dayOfMonthAlarm);

extern void RTC_setCalendarEvent (unsigned int baseAddress,
    unsigned int eventSelect);

extern unsigned long RTC_getCounterValue (unsigned int baseAddress);

extern void RTC_setCounterValue (unsigned int baseAddress,
    unsigned long counterSizeSelect);

extern void RTC_counterPrescaleInit (unsigned int baseAddress,
    unsigned char prescaleSelect,
    unsigned int prescaleClockSelect,
    unsigned int prescaleDivider);

extern void RTC_counterPrescaleHold (unsigned int baseAddress,
    unsigned char prescaleSelect);

extern void RTC_counterPrescaleStart (unsigned int baseAddress,
    unsigned char prescaleSelect);

extern void RTC_definePrescaleEvent (unsigned int baseAddress,
    unsigned char prescaleSelect,
    unsigned char prescaleEventDivider);

extern unsigned char RTC_getPrescaleValue (unsigned int baseAddress,
    unsigned char prescaleSelect);

extern void RTC_setPrescaleValue (unsigned int baseAddress,
    unsigned char prescaleSelect,
    unsigned char prescaleCounterValue);

extern void RTC_enableInterrupt (unsigned int baseAddress,
    unsigned char interruptMask);

extern void RTC_disableInterrupt (unsigned int baseAddress,
    unsigned char interruptMask);

extern unsigned char RTC_getInterruptStatus (unsigned int baseAddress,
    unsigned char interruptFlagMask);

extern void RTC_clearInterrupt (unsigned int baseAddress,
    unsigned char interruptFlagMask);

extern unsigned int RTC_convertBCDToBinary (unsigned int baseAddressu,
    unsigned int valueToConvert);

extern unsigned int RTC_convertBinaryToBCD (unsigned int baseAddress,
    unsigned int valueToConvert);

#endif
