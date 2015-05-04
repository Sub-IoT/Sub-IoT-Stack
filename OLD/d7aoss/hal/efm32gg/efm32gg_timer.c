/*
 *  Created on: October 7, 2014
 *  Authors:
 *  	jeremie@wizzilab.com
 */
#include <stdbool.h>
#include <stdint.h>

#include "em_cmu.h"
#include "em_rtc.h"
#include "em_int.h"

#include <../../framework/log.h>
#include <../../framework/timer.h>
#include <../timer.h>

#include <signal.h>
#include <time.h>

//timer_t timerid;

#define USE_NEW_CONFIGURATION

#define RTC_FREQ    32768


/**************************************************************************//**
 * @brief  Start LFRCO for RTC
 * Starts the low frequency RC oscillator (LFRCO) and routes it to the RTC
 *****************************************************************************/
void startLfxoForRtc(void)
{
    /* Starting LFRCO and waiting until it is stable */
    CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

    /* Routing the LFRCO clock to the RTC */
    CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFXO);
    CMU_ClockEnable(cmuClock_RTC, true);

    /* Set Clock prescaler */
    CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_32);

    /* Enabling clock to the interface of the low energy modules */
    CMU_ClockEnable(cmuClock_CORELE, true);
}

/**************************************************************************//**
 * @brief Enables LFACLK and selects LFXO as clock source for RTC.
 *        Sets up the RTC to count at 1024 Hz.
 *        The counter should not be cleared on a compare match and keep running.
 *        Interrupts should be cleared and enabled.
 *        The counter should run.
 *****************************************************************************/
void hal_timer_init() {
    /* Configuring clocks in the Clock Management Unit (CMU) */
    startLfxoForRtc();

    RTC_Init_TypeDef rtcInit = RTC_INIT_DEFAULT;

    rtcInit.enable   = false;   /* Don't enable RTC after init has run */
    rtcInit.comp0Top = false;   /* Don't clear counter on compare match */
    rtcInit.debugRun = false;   /* Counter shall not keep running during debug halt. */

    /* Initialize the RTC */
    RTC_Init(&rtcInit);

    /* Enable interrupts */
    RTC_IntDisable(RTC_IEN_COMP0);
    RTC_IntClear(RTC_IFC_COMP0);
    NVIC_EnableIRQ(RTC_IRQn);
    INT_Enable();

    /* Start Counter */
    RTC_Enable(true);
}

void hal_timer_enable_interrupt()
{
    /* Enable interrupt */
    RTC_IntEnable(RTC_IEN_COMP0);
}

void hal_timer_disable_interrupt()
{
    RTC_IntDisable(RTC_IEN_COMP0);
}

uint16_t hal_timer_getvalue()
{
    return (uint16_t)RTC->CNT; 
}

void hal_timer_setvalue(uint16_t next_event) {
    // TODO check, only tested with uint32_t
    // the timer counter is 24 bits
    if( next_event & 0xFF000000 )
    {
        next_event = 0x00FFFFFF;
    }

    /* Init counter */
    RTC_CompareSet( 0, next_event );
}

void hal_timer_counter_reset( void )
{
    RTC_CounterReset();
}

void hal_timer_clear_interrupt( void )
{
    RTC_IntClear(RTC_IFC_COMP0);
}
/******************************************************************************
 * @brief RTC Interrupt Handler.
 *****************************************************************************/
void RTC_IRQHandler(void)
{
    timer_completed();
}
