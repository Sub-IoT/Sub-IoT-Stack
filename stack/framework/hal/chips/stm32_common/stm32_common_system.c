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

/*! \file stm32_common_system.c
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */


#include "hwsystem.h"
#include "debug.h"
#include "stm32_device.h"
#include "log.h"


void hw_enter_lowpower_mode(uint8_t mode)
{

//  log_print_string("sleep (mode %i) @ %i", mode, hw_timer_getvalue(0));
  __disable_irq();

  hw_deinit_pheriperals();

  __DSB();
  switch (mode)
  {
    case 0: // sleep mode
      HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      break;
    case 1: // STOP mode
#ifdef FRAMEWORK_DEBUG_ENABLE_SWD
      // enable debugger in stop mode
      __HAL_RCC_DBGMCU_CLK_ENABLE();
      DBGMCU->CR |= DBGMCU_CR_DBG_STOP;
      __HAL_RCC_DBGMCU_CLK_DISABLE( );
#else
      __HAL_FLASH_SLEEP_POWERDOWN_ENABLE(); // TODO test
      // we can't do this in debug mode since DBGMCU the core is always clocked and will not wait for flash to be ready
#endif

      __HAL_RCC_PWR_CLK_ENABLE(); // to be able to change PWR registers
      PWR->CR |= (PWR_CR_ULP & PWR_CR_FWU & PWR_CR_PVDE); // we don't need Vrefint and PVD
      //RCC->CFGR |= RCC_CFGR_STOPWUCK; // use HSI16 after wake up
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
      __HAL_RCC_PWR_CLK_DISABLE();

      assert(EXTI->PR == 0);
      assert((PWR->CSR & PWR_CSR_WUF) == 0);

      HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);

      // after resuming from STOP mode we should reinit the clock config
      stm32_common_mcu_init();
      break;
    default:
      assert(false);
  }

  __enable_irq();
//  log_print_string("wake up @ %i", hw_timer_getvalue(0) );
}

uint64_t hw_get_unique_id()
{
  return (*((uint64_t *)(UID_BASE + 0x04U)) << 32) + *((uint64_t *)(UID_BASE + 0x14U));
}

void hw_busy_wait(int16_t us)
{
  // note: measure this, may switch to timer later if more accuracy is needed.
  uint32_t counter = us * (HAL_RCC_GetSysClockFreq() / 1000000);
  uint32_t n = counter/4;
  __asm volatile (
    " dmb\n"
    " 1:\n"
    " sub %[n], %[n], #1\n"
    " cmp %[n], #0\n"
    " bne 1b"
  : [n] "+r" (n) :: "cc");
}

void hw_reset()
{
  HAL_NVIC_SystemReset();
}

void __hardfault_handler(char* reason) {
  log_print_string("HardFault occured: %s\n", reason);
  assert(false);
}

__attribute__((naked)) void HardFault_Handler();
void HardFault_Handler()
{
  // implemented in asm and as a naked function, to make sure we are not using the stack
  // in case this is originating from a stack overflow.
  // Checks if the stack has overflown, and if yes, reset the stack pointer and call assert()
  __asm volatile (
      "    mov r0,sp\n\t"
      "    ldr r1,=__stack_start\n\t"
      "    cmp r0,r1\n\t"
      "    bcs stack_ok\n\t"
      "    ldr r0,=_estack\n\t"
      "    mov sp,r0\n\t"
      "    ldr r0,=str_overflow\n\t"
      "    mov r1,#1\n\t"
      "    b __hardfault_handler\n\t"
      "stack_ok:\n\t"
      "    ldr r0,=str_hardfault\n\t"
      "    mov r1,#2\n\t"
      "    b __hardfault_handler\n\t"
      "str_overflow:  .asciz \"StackOverflow\"\n\t"
      "str_hardfault: .asciz \"HardFault\"\n\t"
  );
}

