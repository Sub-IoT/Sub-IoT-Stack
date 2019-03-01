/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/HAL/OsIntfL1.c $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*	Low level function for STM32L\n
*/

/*-- Includes --*/
#include "stm32_device.h"
#include "debug.h"
#include "OsIntfL1.h"
#include "log.h"
#include "debug.h"

/*-- MISRA rules --*/
/*
 * Description: "Integral type 'unsigned int' should not be converted to pointer to object type 'RTC_TypeDef *'"
 */
/* parasoft suppress item MISRA2012-RULE-11_4 reason "Conversion in external library (STM32 Cube)"*/

/*-- Local definitions --*/
/*! \cond *//* Local definitions shouldn't be documented */

#ifdef M_DEBUG
	#define M_WATCHDOG				0
#else
	#define M_WATCHDOG				0
#endif

#define M_RESET_FLAG_OBLRST		(1u << 0)
#define M_RESET_FLAG_PINRST		(1u << 1)
#define M_RESET_FLAG_PORRST		(1u << 2)
#define M_RESET_FLAG_SFTRST		(1u << 3)
#define M_RESET_FLAG_IWDGRST	(1u << 4)
#define M_RESET_FLAG_WWDGRST	(1u << 5)
#define M_RESET_FLAG_LPWRRST	(1u << 6)

/*! \endcond *//* End of local definitions */

/*-- Local types --*/
/*! \cond *//* Local types shouldn't be documented */

/*! \endcond *//* End of local types */

/*-- Local data --*/
static IWDG_HandleTypeDef s_iwdgHandle;
static uint32_t s_resetCause;

/*-- Private prototypes --*/
static void SystemClock_Config(void);

/*-- Public functions --*/

/*!
*	Performs OS reset
*/
void OSIntfL1_Init(void)
{
	//assert(HAL_DeInit() == HAL_OK); /* parasoft-suppress MISRA2012-RULE-17_7_a "Checked with M_ASSERT" */
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	//assert(HAL_Init() == HAL_OK); /* parasoft-suppress MISRA2012-RULE-17_7_a "Checked with M_ASSERT" */
	/* Configure the system clock */
	//SystemClock_Config();
	//__HAL_RCC_DBGMCU_CLK_ENABLE();
	/* Watchdog clock is stopped during breakpoint, other peripherals keep running */
	//__HAL_DBGMCU_FREEZE_IWDG();
	/* Enable clocks for watchdog */
	//__HAL_RCC_PWR_CLK_ENABLE(); 
 
    s_resetCause = 0u;
    /*
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST))
    {
        s_resetCause |= M_RESET_FLAG_OBLRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
    {
        s_resetCause |= M_RESET_FLAG_PINRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
    {
        s_resetCause |= M_RESET_FLAG_PORRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
        s_resetCause |= M_RESET_FLAG_SFTRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        s_resetCause |= M_RESET_FLAG_IWDGRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        s_resetCause |= M_RESET_FLAG_WWDGRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
    {
        s_resetCause |= M_RESET_FLAG_LPWRRST;
    }
    */
    /* Clear reset flags */
    //__HAL_RCC_CLEAR_RESET_FLAGS();
        
	/*
	From datasheet:
		These timings are given for a LSI_VALUE kHz clock but the microcontroller�s internal RC frequency can vary from 30
		to 50 kHz.
	*/

	/* Watchdog should also be enabled by option bytes!!! */
#if M_WATCHDOG == 1

	/* LSI should be enabled already, otherwise LSI is automatically enabled when enabling IWDG */
	
	s_iwdgHandle.Instance = IWDG;

	s_iwdgHandle.Init.Prescaler = IWDG_PRESCALER_128;
	s_iwdgHandle.Init.Reload = (500u * 50u) / 128u; /* LSI of maximum 56 KHz and prescaler 128  => 500 ms (Set in relation to AWU delay) */
	s_iwdgHandle.Init.Window = IWDG_WINDOW_DISABLE;

	/* Configure IWDG */
	if(HAL_IWDG_Init(&s_iwdgHandle) != HAL_OK)
	{
		/* Initialization Error */
		assert(false);
	}

	/* Start the IWDG */
	//if(HAL_IWDG_Start(&s_iwdgHandle) != HAL_OK) //already started in init?
	//{
	//	assert(false);
	//}

#endif
}

/*!
*	Kick watchdog
*/
void OSIntfL1_KickWatchDog(void)
{
#if M_WATCHDOG==1
	HAL_IWDG_Refresh(&s_iwdgHandle); /* parasoft-suppress MISRA2012-RULE-17_7_a "Returns always HAL_OK" */
#endif
}

/*!
 * Get the reset cause.
 */
uint32_t OSIntfL1_GetResetCause(void)
{
    return s_resetCause;
}

/*!
 * Clear the reset causes.
 */
void OSIntfL1_ClearResetCause(void)
{
    s_resetCause = 0u;
}


/*!
*	Force reset of the cpu
*/
void OSIntfL1_ForceReset(void)
{
	NVIC_SystemReset(); /* parasoft-suppress MISRA2004-8_1_b "External library function from IAR" */
}

/*-- Private functions --*/

/*!
 * RCC Clock Security System interrupt user callback.
 */
void HAL_RCC_CSSCallback(void)
{
    assert(false);
}

/*!
*	System Clock Configuration
*/
static void SystemClock_Config(void)
{
    // RCC_OscInitTypeDef rccOscInitStruct;
    // RCC_ClkInitTypeDef rccClkInitStruct;
    
	// __HAL_RCC_PWR_CLK_ENABLE();
    // log_print_string("pwrEn\n");
    // rccOscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    // rccOscInitStruct.HSEState = RCC_HSE_ON;
	// rccOscInitStruct.LSIState = RCC_LSI_OFF;
    // rccOscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    // //assert(HAL_RCC_OscConfig(&rccOscInitStruct) == HAL_OK);
    // log_print_string("rccosconf\n");
    // rccClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    // rccClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    // rccClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    // rccClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    // //assert(HAL_RCC_ClockConfig(&rccClkInitStruct, FLASH_LATENCY_0) == HAL_OK);
    // log_print_string("rccclockclonf\n");
    // HAL_RCC_EnableCSS();
    // log_print_string("css\n");
    // /* Systick config */
    // assert(HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000) == HAL_OK);
    // HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    // log_print_string("clksrc\n");
    // /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
