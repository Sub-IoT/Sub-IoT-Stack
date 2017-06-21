/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
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

/*! \file stm32l0xx_timer.c
 *
 */

#include <stdbool.h>
#include <stdint.h>

#include "hwtimer.h"
#include "hwatomic.h"
#include "stm32l0xx_hal.h"

// TODO validate

 static timer_callback_t compare_f = 0x0;
 static timer_callback_t overflow_f = 0x0;
 static bool timer_inited = false;
 static TIM_HandleTypeDef        htim2;
 extern __IO uint32_t uwTick;
// /**************************************************************************//**
//  * @brief Sets up the TIM6 to count at 1024 Hz.
//  *        The counter should not be cleared on a compare match and keep running.
//  *        Interrupts should be cleared and enabled.
//  *        The counter should run.
//  *****************************************************************************/
error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
	if(timer_id >= HWTIMER_NUM)
		return ESIZE;
	if(timer_inited)
		return EALREADY;
	if(frequency != HWTIMER_FREQ_1MS && frequency != HWTIMER_FREQ_32K)
		return EINVAL;

	start_atomic();
	compare_f = compare_callback;
	overflow_f = overflow_callback;
	timer_inited = true;

	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_OC_InitTypeDef sConfigOC;
	
//	htim11.Instance = TIM10;
//	htim11.Init.Prescaler = (uint32_t) (SystemCoreClock / (1024 - 1));
//	htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
//	htim11.Init.Period = (UINT32_C(1) << (8*sizeof(hwtimer_tick_t))) -1;
//	htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//	if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
//	{
//		return FAIL;
//	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
        if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
		 return FAIL;
	}

        if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
	{
		 return FAIL;
	}

	sConfigOC.OCMode = TIM_OCMODE_TIMING;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
        if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		 return FAIL;
	}

	// __HAL_RCC_TIM10_CLK_ENABLE();
	/* Peripheral interrupt init */
	//HAL_NVIC_SetPriority(TIM10_IRQn, 0, 0);
	//HAL_NVIC_EnableIRQ(TIM10_IRQn);

	//__HAL_TIM_DISABLE_IT(&htim11, TIM_IT_CC1 | TIM_IT_UPDATE);
	//__HAL_TIM_CLEAR_FLAG(&htim11, TIM_IT_CC1 | TIM_IT_UPDATE);
	//HAL_TIM_OC_Start_IT(&htim11, TIM_CHANNEL_1);
	//__HAL_TIM_DISABLE_IT(&htim11, TIM_IT_CC1);
	//  __HAL_TIM_ENABLE_IT(&htim11, TIM_IT_UPDATE);
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_IT_CC1 | TIM_IT_UPDATE);
        HAL_TIM_Base_Start_IT(&htim2);

     end_atomic();
    return SUCCESS;
}

hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id)
{
 	if(timer_id >= HWTIMER_NUM || (!timer_inited))
 		return 0;
 	else
 	{
                uint32_t value =__HAL_TIM_GET_COUNTER(&htim2);
 		return value;
 	}
}

error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick )
{
 	if(timer_id >= HWTIMER_NUM)
 		return ESIZE;
 	if(!timer_inited)
 		return EOFF;

 	start_atomic();
        __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);

        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, tick);
 	//HAL_TIM_OC_Start_IT(&htim11, TIM_CHANNEL_1);
        __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC1);
 	//HAL_NVIC_ClearPendingIRQ(TIM6_IRQn);
	//HAL_NVIC_EnableIRQ(TIM6_IRQn);
 	end_atomic();
}

error_t hw_timer_cancel(hwtimer_id_t timer_id)
{
 	if(timer_id >= HWTIMER_NUM)
 		return ESIZE;
 	if(!timer_inited)
 		return EOFF;

 	start_atomic();
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_IT_CC1);
        __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);
 	//HAL_NVIC_ClearPendingIRQ(TIM6_IRQn);
 	//HAL_NVIC_DisableIRQ(TIM6_IRQn);
 	end_atomic();
}

error_t hw_timer_counter_reset(hwtimer_id_t timer_id)
{
 	if(timer_id >= HWTIMER_NUM)
 		return ESIZE;
 	if(!timer_inited)
 		return EOFF;

 	start_atomic();

        __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1 | TIM_IT_UPDATE);
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_IT_CC1 | TIM_IT_UPDATE);
        __HAL_TIM_SET_COUNTER(&htim2, 10);
        __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
 	end_atomic();

}

bool hw_timer_is_overflow_pending(hwtimer_id_t timer_id)
{
     if(timer_id >= HWTIMER_NUM)
 	return false;
     start_atomic();
 	//COMP0 is used to limit thc RTC to 16 bits -> use this one to check
        bool is_pending = __HAL_TIM_GET_FLAG(&htim2, TIM_IT_UPDATE);
     end_atomic();
     return is_pending;
}

bool hw_timer_is_interrupt_pending(hwtimer_id_t timer_id)
{
     if(timer_id >= HWTIMER_NUM)
 	return false;

     start_atomic();
        bool is_pending = __HAL_TIM_GET_FLAG(&htim2, TIM_IT_CC1);
     end_atomic();
     return is_pending;
}

/**
  * @brief  This function configures the TIM2 as a time base source.
  *         The time source is configured  to have 1ms time base with a dedicated
  *         Tick interrupt priority.
  * @note   This function is called  automatically at the beginning of program after
  *         reset by HAL_Init() or at any time when clock is configured, by HAL_RCC_ClockConfig().
  * @param  TickPriority: Tick interrupt priorty.
  * @retval HAL status
  */
//HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
//{
//  RCC_ClkInitTypeDef    clkconfig;
//  uint32_t              uwTimclock = 0;
//  uint32_t              uwPrescalerValue = 0;
//  uint32_t              pFLatency;

//  /*Configure the TIM2 IRQ priority */
//  HAL_NVIC_SetPriority(TIM2_IRQn, TickPriority ,0);

//  /* Enable the TIM2 global Interrupt */
//  HAL_NVIC_EnableIRQ(TIM2_IRQn);

//  /* Enable TIM2 clock */
//  __HAL_RCC_TIM2_CLK_ENABLE();

//  /* Get clock configuration */
//  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

//  /* Compute TIM2 clock */
//  uwTimclock = HAL_RCC_GetPCLK2Freq();

//  /* Compute the prescaler value to have TIM2 counter clock equal to 1MHz */
//  uwPrescalerValue = (uint32_t) ((uwTimclock / 1024) - 1);

//  /* Initialize TIM11 */
//  htim2.Instance = TIM2;

//  /* Initialize TIMx peripheral as follow:
//  + Period = [(TIM2CLK/1000) - 1]. to have a (1/1000) s time base.
//  + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
//  + ClockDivision = 0
//  + Counter direction = Up
//  */
//  htim2.Init.Period = (UINT32_C(1) << (8*sizeof(uint16_t))) -1; //(1000000 / 1000) - 1;
//  htim2.Init.Prescaler = uwPrescalerValue;
//  htim2.Init.ClockDivision = 0;
//  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
//  if(HAL_TIM_Base_Init(&htim2) == HAL_OK)
//  {
//    /* Start the TIM time Base generation in interrupt mode */
//          __HAL_TIM_CLEAR_FLAG(&htim2, TIM_IT_UPDATE);
//    return HAL_TIM_Base_Start_IT(&htim2);
//  }

//  /* Return function status */
//  return HAL_ERROR;
//}

/**
  * @brief  Suspend Tick increment.
  * @note   Disable the tick increment by disabling TIM11 update interrupt.
  * @param  None
  * @retval None
  */
void HAL_SuspendTick(void)
{
  /* Disable TIM11 update Interrupt */
  __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_UPDATE);
}

/**
  * @brief  Resume Tick increment.
  * @note   Enable the tick increment by Enabling TIM11 update interrupt.
  * @param  None
  * @retval None
  */
void HAL_ResumeTick(void)
{
  /* Enable TIM11 Update interrupt */
  __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
}

void HAL_IncTick(void)
{
  uwTick += (htim2.Init.Period + 1);
}

uint32_t HAL_GetTick(void)
{
  return uwTick + __HAL_TIM_GET_COUNTER(&htim2);
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
        if (htim->Instance==TIM2)
	{
                __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);
		if(compare_f != 0x0)
				compare_f();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
        if (htim->Instance==TIM2)
	{
                uwTick += (htim2.Init.Period + 1);
		if(overflow_f != 0x0)
			overflow_f();
	}
}


void TIM11_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim2);
}


