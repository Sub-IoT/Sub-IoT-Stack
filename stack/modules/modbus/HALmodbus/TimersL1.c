/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/HAL/TimersL1.c $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*	Lower level function for timer \n
*/

/*-- Includes --*/
#include "stm32_device.h"
#include <stddef.h>
#include "TimersL1.h"
#include "debug.h"

/*-- Local definitions --*/
/*! \cond *//* Local definitions shouldn't be documented */

/* Define maximum number of call backs */
#define	M_TIMERCALLBACKS	4u

/*! \endcond *//* End of local definitions */

/*-- Local types --*/
/*! \cond *//* Local types shouldn't be documented */

typedef struct stTimerCallBack
{
    void (*m_pCallBack)(void);
    uint32_t m_Period;
    uint32_t m_ElapsedTime;
} timerCallBack_t;

/*! \endcond *//* End of local types */


/*-- Local data --*/
static timerCallBack_t s_aTimerCallBack[M_TIMERCALLBACKS];

/*-- Private prototypes --*/
void SysTick_Handler(void);

/*-- Public functions --*/


/*!
*	Performs total timers reset (Layer 1)
*/
void TimersL1_Init(void)
{
    uint32_t itIndex;
	
    for (itIndex = 0u; itIndex < M_TIMERCALLBACKS; ++itIndex)
    {
        s_aTimerCallBack[itIndex].m_pCallBack = NULL;
    }
	
	/* SysTick timer is started in OsIntfl (HAL_Init) */
}

/*!
*	Retrieve system time
*
*	\return 	Numbers of milliseconds elapsed since power-on, in 32-bit
*/
uint32_t TimersL1_GetSystemTime(void)
{
	/* interrupt safe because of 32-bit processor */
    return HAL_GetTick();
}

/*!
*	Enable SysTick counter/interrupt
*/
void TimersL1_Enable(void)
{
	HAL_ResumeTick();
}

/*!
*	Disable SysTick counter/interrupt
*/
void TimersL1_Disable(void)
{
	HAL_SuspendTick();
}

/*!
*	Adds a callback to the timer
*	Caution: assert when no callback slot are available
*
*	\param[in]		pCallBackFn		Pointer to callback function
*	\param[in]		CallPeriod		Period in ms to call fucntion
*/
void TimersL1_AddCallBack(void (*pCallBackFn)(void), uint32_t callPeriod)
{
	uint8_t callBack;
    for(callBack=0u; (callBack < M_TIMERCALLBACKS) && (s_aTimerCallBack[callBack].m_pCallBack != (void*)0u); ++callBack)
    {
    }

    /* still got room? */
    assert( callBack < M_TIMERCALLBACKS);

	s_aTimerCallBack[callBack].m_pCallBack = pCallBackFn;
	s_aTimerCallBack[callBack].m_Period = callPeriod;
	s_aTimerCallBack[callBack].m_ElapsedTime = callPeriod;
}

/*!
*	Delay for milliseconds
*	Caution: Blocking
*
*	\param[in]		delayMs		Delay in milliseconds
*/
void TimersL1_Delay(uint32_t delayMs)
{
	/* Make sure to wait at least delayMs */
	HAL_Delay(delayMs + 1u);
}


/*-- Private functions --*/

/*! Setup Systick compare interrupt handler */
void SysTick_Handler(void) /* parasoft-suppress MISRA2004-8_10 "Interrupt handler prototype is defined in assembler code" */
{

	/* Increment millisecond counter in CubeMX HAL */
	HAL_IncTick();
	
    /* call clients */
	for(uint8_t callBack = 0u; (callBack < M_TIMERCALLBACKS) && (s_aTimerCallBack[callBack].m_pCallBack != (void*)0u); callBack++)
	{
		s_aTimerCallBack[callBack].m_ElapsedTime--;
		if (s_aTimerCallBack[callBack].m_ElapsedTime == 0u)
		{
			s_aTimerCallBack[callBack].m_ElapsedTime += s_aTimerCallBack[callBack].m_Period;
			s_aTimerCallBack[callBack].m_pCallBack();
		}
	}
}
