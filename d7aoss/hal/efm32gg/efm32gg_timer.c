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
 * @brief Enables LFACLK and selects LFXO as clock source for RTC
 *        Sets up the RTC to generate an interrupt every minute.
 *****************************************************************************/
void hal_timer_init() {
    /* Configuring clocks in the Clock Management Unit (CMU) */
    startLfxoForRtc();

    RTC_Init_TypeDef rtcInit = RTC_INIT_DEFAULT;

    rtcInit.enable   = false;      /* Enable RTC after init has run */
    rtcInit.comp0Top = true;      /* Clear counter on compare match */
    rtcInit.debugRun = true;     /* Counter shall keep running during debug halt. */

    /* Setting the reset value of the RTC counter */
    RTC_CompareSet(0, 65536);

    /* Initialize the RTC */
    RTC_Init(&rtcInit);

    /* Enable interrupts */
    RTC_IntClear(RTC_IFC_COMP0);
    RTC_IntClear(RTC_IFC_COMP1);
    NVIC_EnableIRQ(RTC_IRQn);
    RTC_IntEnable(RTC_IEN_COMP0);
    RTC_IntDisable(RTC_IEN_COMP1);
    INT_Enable();

    /* Start Counter */
    RTC_Enable(true);
}

void hal_timer_enable_interrupt() {
    /* Enable interrupt */
    RTC_IntEnable(RTC_IEN_COMP1);
    //INT_Enable();
}

void hal_timer_disable_interrupt() {
    RTC_IntDisable(RTC_IEN_COMP1);
}

uint16_t hal_timer_getvalue() {
    return (uint16_t)RTC->CNT;
}

void hal_timer_setvalue(uint16_t next_event) {
  
    /* Init counter */
    RTC_CompareSet(1, next_event - 1 );
}

	/**************************************************************************//**
 * @brief RTC Interrupt Handler.
 *****************************************************************************/
void RTC_IRQHandler(void)
{
    /* Clear interrupt source */
    if(RTC->IF&RTC_IFC_COMP0)
    {
        RTC_IntClear(RTC_IFC_COMP0);
    }
    else if(RTC->IF&RTC_IFC_COMP1)
    {
        RTC_IntClear(RTC_IFC_COMP1);
        timer_completed();
    }
}
