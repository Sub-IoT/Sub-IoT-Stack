/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2018 University of Antwerp
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

/*! \file stm32_common_timer.c
 *
 */

#include <stdbool.h>
#include <stdint.h>

#include "hwtimer.h"
#include "hwatomic.h"
#include "stm32_device.h"
#include "debug.h"
#include "errors.h"

#define HWTIMER_NUM 1

// TODO define timers in ports.h
#if defined(STM32L0)
  #define TIMER_INSTANCE TIM22
  #define TIMER_IRQ TIM22_IRQn
  #define TIMER_ISR TIM22_IRQHandler
#elif defined(STM32L1)
  #define TIMER_INSTANCE TIM10
  #define TIMER_IRQ TIM10_IRQn
  #define TIMER_ISR TIM10_IRQHandler
#else
  #error "Family not supported"
#endif

// TODO validate

 static timer_callback_t compare_f = 0x0;
 static timer_callback_t overflow_f = 0x0;
 static bool timer_inited = false;
 static TIM_HandleTypeDef timer;


// /*****************************************************************************
//  * @brief Sets up the TIM22 to count at 1024 Hz, driven by the 32.768 kHz LSE
//  *        The counter should not be cleared on a compare match and keep running.
//  *        Interrupts should be cleared and enabled.
//  *        The counter should run.
//  *****************************************************************************/
error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
  // TODO using only one timer for now
	if(timer_id >= HWTIMER_NUM)
		return ESIZE;
	if(timer_inited)
		return EALREADY;
	if(frequency != HWTIMER_FREQ_1MS && frequency != HWTIMER_FREQ_32K)
		return EINVAL;

  TIM_ClockConfigTypeDef clock_source_config;
  TIM_MasterConfigTypeDef master_config;

  start_atomic();
	compare_f = compare_callback;
	overflow_f = overflow_callback;
	timer_inited = true;

#if defined(STM32L0)
  __HAL_RCC_TIM22_CLK_ENABLE();
#elif defined(STM32L1)
  __HAL_RCC_TIM10_CLK_ENABLE();
#endif

  timer.Instance = TIMER_INSTANCE;
  timer.Init.Prescaler = 31;
  timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  timer.Init.Period = 0xFFFF;
  timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  assert(HAL_TIM_Base_Init(&timer) == HAL_OK);

  clock_source_config.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
  clock_source_config.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
  clock_source_config.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
  clock_source_config.ClockFilter = 0;
  assert(HAL_TIM_ConfigClockSource(&timer, &clock_source_config) == HAL_OK);

  master_config.MasterOutputTrigger = TIM_TRGO_RESET;
  master_config.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  assert(HAL_TIMEx_MasterConfigSynchronization(&timer, &master_config) == HAL_OK);
#if defined(STM32L0)
  assert(HAL_TIMEx_RemapConfig(&timer, TIM22_ETR_LSE) == HAL_OK);
#elif defined(STM32L1)
  assert(HAL_TIMEx_RemapConfig(&timer, TIM_TIM10_ETR_LSE) == HAL_OK);
#endif

  __HAL_TIM_ENABLE_IT(&timer, TIM_IT_UPDATE);
  __HAL_TIM_ENABLE(&timer);

  // make sure we only get an update interrupt on overflow, and not on for instance reset of CC
  __HAL_TIM_URS_ENABLE(&timer);
  __HAL_TIM_CLEAR_FLAG(&timer, TIM_SR_UIF);
  HAL_NVIC_SetPriority(TIMER_IRQ, 0, 0);
  HAL_NVIC_EnableIRQ(TIMER_IRQ);
  end_atomic();
  return SUCCESS;
}

hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id)
{
 	if(timer_id >= HWTIMER_NUM || (!timer_inited))
 		return 0;
 	else
 	{
    uint32_t value =__HAL_TIM_GET_COUNTER(&timer);
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
    __HAL_TIM_DISABLE_IT(&timer, TIM_IT_CC1);
    __HAL_TIM_SET_COMPARE(&timer, TIM_CHANNEL_1, tick);
    __HAL_TIM_ENABLE_IT(&timer, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(&timer, TIM_IT_CC1);
    HAL_NVIC_ClearPendingIRQ(TIMER_IRQ);
    HAL_NVIC_EnableIRQ(TIMER_IRQ);
 	end_atomic();

}

error_t hw_timer_cancel(hwtimer_id_t timer_id)
{
 	if(timer_id >= HWTIMER_NUM)
 		return ESIZE;
 	if(!timer_inited)
 		return EOFF;

 	start_atomic();
    __HAL_TIM_DISABLE_IT(&timer, TIM_IT_CC1);
    __HAL_TIM_CLEAR_FLAG(&timer, TIM_IT_CC1);
    HAL_NVIC_ClearPendingIRQ(TIMER_IRQ);
 	end_atomic();
}

error_t hw_timer_counter_reset(hwtimer_id_t timer_id)
{
 	if(timer_id >= HWTIMER_NUM)
 		return ESIZE;
 	if(!timer_inited)
 		return EOFF;

 	start_atomic();
    __HAL_TIM_DISABLE_IT(&timer, TIM_IT_CC1 | TIM_IT_UPDATE);
    __HAL_TIM_CLEAR_FLAG(&timer, TIM_IT_CC1 | TIM_IT_UPDATE);
    __HAL_TIM_SET_COUNTER(&timer, 10); // TODO 10??
    __HAL_TIM_ENABLE_IT(&timer, TIM_IT_UPDATE);
 	end_atomic();

  return SUCCESS;
}

bool hw_timer_is_overflow_pending(hwtimer_id_t timer_id)
{
  if(timer_id >= HWTIMER_NUM)
    return false;

  start_atomic();
    bool is_pending = __HAL_TIM_GET_FLAG(&timer, TIM_IT_UPDATE);
  end_atomic();

  return is_pending;
}

bool hw_timer_is_interrupt_pending(hwtimer_id_t timer_id)
{
  if(timer_id >= HWTIMER_NUM)
    return false;

  start_atomic();
    bool is_pending = __HAL_TIM_GET_FLAG(&timer, TIM_IT_CC1);
  end_atomic();

  return is_pending;
}


void TIMER_ISR(void)
{
  // We are not using HAL_TIM_IRQHandler() here to reduce interrupt latency
  // first check for overflow ...
  if(__HAL_TIM_GET_FLAG(&timer, TIM_FLAG_UPDATE) != RESET)
  {
    if(__HAL_TIM_GET_IT_SOURCE(&timer, TIM_IT_UPDATE) !=RESET)
    {
      __HAL_TIM_CLEAR_IT(&timer, TIM_IT_UPDATE);
      if(overflow_f != 0x0)
        overflow_f();
    }
  }

  // ... and then for compare value
  if(__HAL_TIM_GET_FLAG(&timer, TIM_FLAG_CC1) != RESET)
  {
    if(__HAL_TIM_GET_IT_SOURCE(&timer, TIM_IT_CC1) !=RESET)
    {
      __HAL_TIM_DISABLE_IT(&timer, TIM_IT_CC1);
      __HAL_TIM_CLEAR_IT(&timer, TIM_IT_CC1);
      if(compare_f != 0x0)
          compare_f();
    }
  }
}


