/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "msp430_addresses.h"
#include "../rtc.h"
//#include "inc/hw_memmap.h"
#include "driverlib/5xx_6xx/rtc.h"


// Currrently 1 sec intervals
void rtc_init_counter_mode()
{
//    //Initialize Counter Mode of RTC_A
//    /*
//     * Base Address of the RTC_A
//     * Use Prescaler 1 as source for counter
//     * Specify counter as 8 bits, which asserts an interrupt for an overflow
//     */
//    RTC_counterInit(__MSP430_BASEADDRESS_RTC__,
//        RTC_CLOCKSELECT_RT1PS,
//        RTC_COUNTERSIZE_8BIT);
//
//    //Initialize Prescalers
//    /*
//     * Base Address of the RTC_A
//     * Specify Initialization of Prescaler 0
//     * Use ACLK as source to prescaler
//     * Divide source by 8 for this prescaler
//     */
//    RTC_counterPrescaleInit(__MSP430_BASEADDRESS_RTC__,
//        RTC_PRESCALE_0,
//        RTC_PSCLOCKSELECT_ACLK,
//        RTC_PSDIVIDER_8);
//        //RTC_PSDIVIDER_2);
//
//    /*
//     * Base Address of the RTC_A
//     * Specify Initialization of Prescaler 1
//     * Use Prescaler 0 as source to prescaler
//     * Divide source by 16 for this prescaler
//     */
//    RTC_counterPrescaleInit(__MSP430_BASEADDRESS_RTC__,
//        RTC_PRESCALE_1,
//        RTC_PSCLOCKSELECT_RT0PS,
//        RTC_PSDIVIDER_16);
//        //RTC_PSDIVIDER_RTC_PSDIVIDER_256);

}

void rtc_enable_interrupt()
{
    //Enable interrupt for counter overflow
//    RTC_enableInterrupt(__MSP430_BASEADDRESS_RTC__,
//        RTCTEVIE);
}

void rtc_disable_interrupt()
{
    //Enable interrupt for counter overflow
//    RTC_disableInterrupt(__MSP430_BASEADDRESS_RTC__,
//        RTCTEVIE);
}

void rtc_start()
{
    //Start RTC Clock
//    RTC_startClock(__MSP430_BASEADDRESS_RTC__);
}

void rtc_stop()
{
    //Start RTC Clock
//    RTC_holdClock(__MSP430_BASEADDRESS_RTC__);
}

