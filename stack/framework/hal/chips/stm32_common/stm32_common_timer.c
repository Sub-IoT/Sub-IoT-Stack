/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
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

#include "platform_defs.h"

#ifndef PLATFORM_USE_RTC

#include <stdbool.h>
#include <stdint.h>

#include "hwtimer.h"
#include "hwatomic.h"
#include "stm32_device.h"
#include "debug.h"
#include "errors.h"
#include "log.h"
#include "callstack.h"

#define HWTIMER_NUM 1

// TODO define timers in ports.h
#if defined(STM32L0)
  #define TIMER_INSTANCE LPTIM1
  #define TIMER_IRQ LPTIM1_IRQn
  #define TIMER_ISR LPTIM1_IRQHandler
#elif defined(STM32L1)
  #define TIMER_INSTANCE TIM10
  #define TIMER_IRQ TIM10_IRQn
  #define TIMER_ISR TIM10_IRQHandler
#else
  #error "Family not supported"
#endif

static timer_callback_t compare_f = 0x0;
static timer_callback_t overflow_f = 0x0;
static bool timer_inited = false;
#if defined(STM32L0)
  static LPTIM_HandleTypeDef timer;
  static volatile bool cmp_reg_write_pending = false;
  static volatile bool ready_for_trigger = false;
#elif defined(STM32L1)
  static TIM_HandleTypeDef timer;
#endif

// Sets up a timer to count at 1024 Hz, driven by the 32.768 kHz LSE
// The timer is running continuously. On STM32L0 the LPTIM1 is used,
// on STM32L1 we use TIM10 a general purpose
error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
  // TODO using only one timer for now
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

  #if defined(STM32L0)
  // set LPTIM1 clock source to LSE
  RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;
  RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
  RCC_PeriphClkInit.LptimClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
  assert(HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit) == HAL_OK);
  #endif

#if defined(STM32L0)
  __HAL_RCC_LPTIM1_CLK_ENABLE();
  __HAL_RCC_LPTIM1_FORCE_RESET();
  __HAL_RCC_LPTIM1_RELEASE_RESET();

  timer.Instance = TIMER_INSTANCE;
  timer.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  timer.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32; // TODO
  timer.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  timer.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  timer.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  assert(HAL_LPTIM_Init(&timer) == HAL_OK);

  __HAL_LPTIM_ENABLE(&timer);
  __HAL_LPTIM_AUTORELOAD_SET(&timer, 0xFFFF);
  __HAL_LPTIM_START_CONTINUOUS(&timer);

  __HAL_LPTIM_ENABLE_IT(&timer, LPTIM_IT_ARRM);
  __HAL_LPTIM_ENABLE_IT(&timer, LPTIM_IT_CMPOK);
#elif defined(STM32L1)
  TIM_ClockConfigTypeDef clock_source_config;
  TIM_MasterConfigTypeDef master_config;
  __HAL_RCC_TIM10_CLK_ENABLE();
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
  assert(HAL_TIMEx_RemapConfig(&timer, TIM_TIM10_ETR_LSE) == HAL_OK);
  __HAL_TIM_ENABLE_IT(&timer, TIM_IT_UPDATE);
  __HAL_TIM_ENABLE(&timer);

  // make sure we only get an update interrupt on overflow, and not on for instance reset of CC
  __HAL_TIM_URS_ENABLE(&timer);
  __HAL_TIM_CLEAR_FLAG(&timer, TIM_SR_UIF);
#endif
  HAL_NVIC_SetPriority(TIMER_IRQ, 0, 0);
  HAL_NVIC_ClearPendingIRQ(TIMER_IRQ);
  HAL_NVIC_EnableIRQ(TIMER_IRQ);
  end_atomic();
  return SUCCESS;
}

const hwtimer_info_t* hw_timer_get_info(hwtimer_id_t timer_id)
{
    if(timer_id >= HWTIMER_NUM)
      return NULL;

    static const hwtimer_info_t timer_info = {
#if defined(STM32L0)
      .min_delay_ticks = 5, // for LPTIMER we need a minimal delay
#elif defined(STM32L1)
      .min_delay_ticks = 0,
#endif
    };

    return &timer_info;
}

hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id)
{
 	if(timer_id >= HWTIMER_NUM || (!timer_inited))
 		return 0;
 	else
 	{
#if defined(STM32L0)
    // only valid value until 2 consecutive reads return the same value, see reference manuel
    uint32_t value = LPTIM1->CNT;
    while(value != LPTIM1->CNT) {
      value = LPTIM1->CNT;
    }
#elif defined(STM32L1)
    uint32_t value =__HAL_TIM_GET_COUNTER(&timer);
#endif
 		return value;
 	}
}

error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick )
{
 	if(timer_id >= HWTIMER_NUM)
 		return ESIZE;
 	if(!timer_inited)
 		return EOFF;
  //don't enable the interrupt while waiting to write a new compare value
  ready_for_trigger = false;
  if(in_atomic())
  {
    log_print_error_string("ILLEGAL TIMER USAGE: hw_timer_schedule should not be used in atomic context!");
    assert(cmp_reg_write_pending == false); // if we currently are in atomic context and cmp_reg_write_is_pending is true we have an infinite loop on the instruction below.
  }
  while(cmp_reg_write_pending); // prev write operation is pending, writing again before may give unpredicatable results (see datasheet), so block here
  ready_for_trigger = true;
  start_atomic();
#if defined(STM32L0)
      cmp_reg_write_pending = true; // cleared in ISR
    __HAL_LPTIM_DISABLE_IT(&timer, LPTIM_IT_CMPM);
    __HAL_LPTIM_CLEAR_FLAG(&timer, LPTIM_FLAG_CMPM);
    __HAL_LPTIM_COMPARE_SET(&timer, tick - 1);
#elif defined(STM32L1)
    __HAL_TIM_DISABLE_IT(&timer, TIM_IT_CC1);
    __HAL_TIM_SET_COMPARE(&timer, TIM_CHANNEL_1, tick);
    __HAL_TIM_ENABLE_IT(&timer, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(&timer, TIM_IT_CC1);
#endif
    HAL_NVIC_ClearPendingIRQ(TIMER_IRQ);
  end_atomic();
  
  return SUCCESS;
}

error_t hw_timer_cancel(hwtimer_id_t timer_id)
{
 	if(timer_id >= HWTIMER_NUM)
 		return ESIZE;
 	if(!timer_inited)
 		return EOFF;

 	start_atomic();
#if defined(STM32L0)
    __HAL_LPTIM_DISABLE_IT(&timer, LPTIM_IT_CMPM);
    __HAL_LPTIM_CLEAR_FLAG(&timer, LPTIM_IT_CMPM);
    ready_for_trigger = false;
#elif defined(STM32L1)
    __HAL_TIM_DISABLE_IT(&timer, TIM_IT_CC1);
    __HAL_TIM_CLEAR_FLAG(&timer, TIM_IT_CC1);
#endif
    HAL_NVIC_ClearPendingIRQ(TIMER_IRQ);
 	end_atomic();

  return SUCCESS;
}

error_t hw_timer_counter_reset(hwtimer_id_t timer_id)
{
 	if(timer_id >= HWTIMER_NUM)
 		return ESIZE;
 	if(!timer_inited)
 		return EOFF;

 	start_atomic();
#if defined(STM32L0)
    __HAL_LPTIM_DISABLE_IT(&timer, LPTIM_IT_CMPM | LPTIM_IT_ARRM);
    __HAL_LPTIM_CLEAR_FLAG(&timer, LPTIM_FLAG_CMPM | LPTIM_FLAG_ARRM);
    timer.Instance->CNT = 0;
    __HAL_LPTIM_ENABLE_IT(&timer, LPTIM_IT_CMPM | LPTIM_IT_ARRM);
#elif defined(STM32L1)
    __HAL_TIM_DISABLE_IT(&timer, TIM_IT_CC1 | TIM_IT_UPDATE);
    __HAL_TIM_CLEAR_FLAG(&timer, TIM_IT_CC1 | TIM_IT_UPDATE);
    __HAL_TIM_SET_COUNTER(&timer, 10); // TODO 10??
    __HAL_TIM_ENABLE_IT(&timer, TIM_IT_CC1 | TIM_IT_UPDATE);
#endif
 	end_atomic();

  return SUCCESS;
}

bool hw_timer_is_overflow_pending(hwtimer_id_t timer_id)
{
  if(timer_id >= HWTIMER_NUM)
    return false;

  start_atomic();
#if defined(STM32L0)
  bool is_pending = __HAL_LPTIM_GET_FLAG(&timer, LPTIM_FLAG_ARRM);
#elif defined(STM32L1)
    bool is_pending = __HAL_TIM_GET_FLAG(&timer, TIM_FLAG_UPDATE);
#endif
  end_atomic();

  return is_pending;
}

bool hw_timer_is_interrupt_pending(hwtimer_id_t timer_id)
{
  if(timer_id >= HWTIMER_NUM)
    return false;

  start_atomic();
#if defined(STM32L0)
    bool is_pending = __HAL_LPTIM_GET_FLAG(&timer, LPTIM_FLAG_CMPM);
#elif defined(STM32L1)
    bool is_pending = __HAL_TIM_GET_FLAG(&timer, TIM_FLAG_CC1);
#endif
  end_atomic();

  return is_pending;
}

#ifdef FRAMEWORK_USE_CALLSTACK
void TIMER_ISR_real(volatile uintptr_t sp)
{
  callstack_stack_pointer = sp;
#else
void TIMER_ISR(void)
{
#endif
  // We are not using HAL_TIM_IRQHandler() here to reduce interrupt latency
#if defined(STM32L0)
  if(__HAL_LPTIM_GET_FLAG(&timer, LPTIM_FLAG_CMPOK) != RESET)
  {
     // compare register write done, new writes are allowed now
     __HAL_LPTIM_CLEAR_FLAG(&timer, LPTIM_FLAG_CMPOK);
     __HAL_LPTIM_CLEAR_FLAG(&timer, LPTIM_IT_CMPM);
     // When timer event is canceled or when another event is already waiting to be written, don't enable the trigger
     if(ready_for_trigger) {
      __HAL_LPTIM_ENABLE_IT(&timer, LPTIM_IT_CMPM);
     }
     cmp_reg_write_pending = false;
  }

  // first check for overflow ...
  if(__HAL_LPTIM_GET_FLAG(&timer, LPTIM_FLAG_ARRM) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(&timer, LPTIM_FLAG_ARRM) !=RESET)
    {
      __HAL_LPTIM_CLEAR_FLAG(&timer, LPTIM_IT_ARRM);
      if(overflow_f != 0x0)
        overflow_f();
    }
  }

  // ... and then for compare value
  if(__HAL_LPTIM_GET_FLAG(&timer, LPTIM_IT_CMPM) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(&timer, LPTIM_IT_CMPM) !=RESET)
    {
      //__HAL_LPTIM_DISABLE_IT(&timer, LPTIM_IT_CMPM);
      __HAL_LPTIM_CLEAR_FLAG(&timer, LPTIM_IT_CMPM);
      // do not trigger higher layer when compare register isn't fully written yet, another write is in queue or when the timer is canceled
      if((compare_f != 0x0) && !cmp_reg_write_pending && ready_for_trigger) {
          compare_f();
      }
    }
  }

  // clear autoreload register update OK if set
  if(__HAL_LPTIM_GET_FLAG(&timer, LPTIM_FLAG_ARROK) != RESET)
  {
    __HAL_LPTIM_CLEAR_FLAG(&timer, LPTIM_IT_ARROK);
  }

#elif defined(STM32L1)
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
#endif
}

#ifdef FRAMEWORK_USE_CALLSTACK
// This assembly function is executed before the real timer interrupt.
// It puts the stack pointer in register 0 and calls the real timer interrupt
// It checks if it has to take the main stack pointer or the process stack pointer
void __attribute__((__naked__)) TIMER_ISR(void)
{
            __asm volatile("MRS R0, MSP\n"                \
                           "MOV R1, LR\n"                 \
                           "MOV R2, #4\n"                 \
                           "TST R1, R2\n"                 \
                           "BEQ TIMER_ISR_call_real\n"    \
                           "MRS R0, PSP\n"                \
                        "TIMER_ISR_call_real:\n"          \
                           " ldr r2,  =TIMER_ISR_real\n"  \
                           " bx  r2  \n"                  \
                           :                              \
                           :                              \
                           : "r0", "r1", "r2");
}
#endif
#endif
