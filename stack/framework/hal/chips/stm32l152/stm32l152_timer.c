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

/*! \file stm32l152_timer.c
 *
 */

#include <stdbool.h>
#include <stdint.h>

#include "hwtimer.h"
#include "hwatomic.h"
#include "stm32l1xx_hal.h"



 static timer_callback_t compare_f = 0x0;
 static timer_callback_t overflow_f = 0x0;
 static bool timer_inited = false;
 static TIM_HandleTypeDef htim10;
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
	
	htim10.Instance = TIM10;
	htim10.Init.Prescaler = (uint32_t) (SystemCoreClock / (1024 - 1));
	htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim10.Init.Period = (UINT32_C(1) << (8*sizeof(hwtimer_tick_t))) -1;
	htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
	{
		return FAIL;
	}

	   sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	   if (HAL_TIM_ConfigClockSource(&htim10, &sClockSourceConfig) != HAL_OK)
	   {
		     return FAIL;
	   }

	   if (HAL_TIM_OC_Init(&htim10) != HAL_OK)
	   {
		     return FAIL;
	   }

	   sConfigOC.OCMode = TIM_OCMODE_TIMING;
	   sConfigOC.Pulse = 0;
	   sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	   sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	   if (HAL_TIM_OC_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	   {
		     return FAIL;
	   }

	// __HAL_RCC_TIM10_CLK_ENABLE();
	/* Peripheral interrupt init */
	//HAL_NVIC_SetPriority(TIM10_IRQn, 0, 0);
	//HAL_NVIC_EnableIRQ(TIM10_IRQn);

	//__HAL_TIM_DISABLE_IT(&htim10, TIM_IT_CC1 | TIM_IT_UPDATE);
	//__HAL_TIM_CLEAR_FLAG(&htim10, TIM_IT_CC1 | TIM_IT_UPDATE);
	//HAL_TIM_OC_Start_IT(&htim10, TIM_CHANNEL_1);
	//__HAL_TIM_DISABLE_IT(&htim10, TIM_IT_CC1);
	//  __HAL_TIM_ENABLE_IT(&htim10, TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_FLAG(&htim10, TIM_IT_CC1 | TIM_IT_UPDATE);
	HAL_TIM_Base_Start_IT(&htim10);



	//





     end_atomic();
    return SUCCESS;
}

hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id)
{
 	if(timer_id >= HWTIMER_NUM || (!timer_inited))
 		return 0;
 	else
 	{
 		uint32_t value =__HAL_TIM_GET_COUNTER(&htim10);
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
 	__HAL_TIM_DISABLE_IT(&htim10, TIM_IT_CC1);

 	__HAL_TIM_SET_COMPARE(&htim10, TIM_CHANNEL_1, tick);
 	//HAL_TIM_OC_Start_IT(&htim10, TIM_CHANNEL_1);
 	__HAL_TIM_ENABLE_IT(&htim10, TIM_IT_CC1);
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
 	__HAL_TIM_CLEAR_FLAG(&htim10, TIM_IT_CC1);
 	__HAL_TIM_DISABLE_IT(&htim10, TIM_IT_CC1);
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

 	__HAL_TIM_DISABLE_IT(&htim10, TIM_IT_CC1 | TIM_IT_UPDATE);
 	__HAL_TIM_CLEAR_FLAG(&htim10, TIM_IT_CC1 | TIM_IT_UPDATE);
 	__HAL_TIM_SET_COUNTER(&htim10, 10);
	__HAL_TIM_ENABLE_IT(&htim10, TIM_IT_UPDATE);
 	end_atomic();

}

bool hw_timer_is_overflow_pending(hwtimer_id_t timer_id)
{
     if(timer_id >= HWTIMER_NUM)
 	return false;
     start_atomic();
 	//COMP0 is used to limit thc RTC to 16 bits -> use this one to check
 	bool is_pending = __HAL_TIM_GET_FLAG(&htim10, TIM_IT_UPDATE);
     end_atomic();
     return is_pending;
}

bool hw_timer_is_interrupt_pending(hwtimer_id_t timer_id)
{
     if(timer_id >= HWTIMER_NUM)
 	return false;

     start_atomic();
 	bool is_pending = __HAL_TIM_GET_FLAG(&htim10, TIM_IT_CC1);
     end_atomic();
     return is_pending;
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance==TIM10)
		{

			__HAL_TIM_DISABLE_IT(&htim10, TIM_IT_CC1);
			if(compare_f != 0x0)
			 		compare_f();
		}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance==TIM10)
	{
		if(overflow_f != 0x0)
			overflow_f();
	}
}


void TIM10_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim10);
}


