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

/*! \file stm32_common_mcu.c
 *  \author glenn.ergeerts@uantwerpen.be
 */

#include "stm32_device.h"

#include "debug.h"

#include "timer.h"
#include "hwsystem.h"
#include "platform_defs.h"
#include "log.h"

#define DPRINT(...)
//#define DPRINT(...) log_print_string(__VA_ARGS__)

const static RCC_ClkInitTypeDef RCC_ClkInitStruct_active = {
  .ClockType        = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2),
  .SYSCLKSource     = RCC_SYSCLKSOURCE_PLLCLK,
  .AHBCLKDivider    = RCC_SYSCLK_DIV1,
  .APB1CLKDivider   = RCC_HCLK_DIV1,
};
const static RCC_ClkInitTypeDef RCC_ClkInitStruct_sleep  = {
  .ClockType        = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2,
  .SYSCLKSource     = RCC_SYSCLKSOURCE_MSI,
  .AHBCLKDivider    = RCC_SYSCLK_DIV1,
  .APB1CLKDivider   = RCC_HCLK_DIV1,
  .APB2CLKDivider   = RCC_HCLK_DIV1
};
const static RCC_OscInitTypeDef RCC_OSC_active_hsi = {
  .OscillatorType     = RCC_OSCILLATORTYPE_HSI,
  .HSIState            = RCC_HSI_ON,
  .HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
  .PLL.PLLState        = RCC_PLL_ON,
  .PLL.PLLSource       = RCC_PLLSOURCE_HSI,
  .PLL.PLLMUL          = RCC_PLL_MUL6,
  .PLL.PLLDIV          = RCC_PLL_DIV3
};
const static RCC_OscInitTypeDef RCC_OSC_active_lse = {
  .OscillatorType      = RCC_OSCILLATORTYPE_LSE,
  .LSEState            = RCC_LSE_ON
};
const static RCC_OscInitTypeDef RCC_OSC_active_msi = {
  .OscillatorType      = RCC_OSCILLATORTYPE_MSI,
  .MSIState            = RCC_MSI_OFF
};
const static RCC_OscInitTypeDef RCC_OSC_sleep_msi = {
  .OscillatorType       = RCC_OSCILLATORTYPE_MSI,
  .MSIState             = RCC_MSI_ON,
  .MSICalibrationValue  = 0,
  .MSIClockRange        = RCC_MSIRANGE_0,
  .PLL.PLLState         = RCC_PLL_NONE
};
const static RCC_OscInitTypeDef RCC_OSC_sleep_hsi = {
  .OscillatorType     = RCC_OSCILLATORTYPE_HSI,
  .HSIState            = RCC_HSI_OFF,
  .PLL.PLLState        = RCC_PLL_OFF
};

static void init_clock(void)
{
  // using 32MHz clock based on HSI+PLL, use 32k LSE for timer
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOS) != RESET) {};

#if defined(STM32L0)
  // TODO not defined on STM32L1
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);
#endif
  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);

  /* Set every clock individually, you cannot set them all together */
  assert(HAL_RCC_OscConfig(&RCC_OSC_active_hsi) == HAL_OK);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2  clocks dividers */
  assert(HAL_RCC_ClockConfig(&RCC_ClkInitStruct_active, FLASH_LATENCY_1) == HAL_OK);

  /* Enable LSE for timer */
  assert(HAL_RCC_OscConfig(&RCC_OSC_active_lse) == HAL_OK);

#ifdef FRAMEWORK_DEBUG_ENABLE_SWD
    __HAL_RCC_DBGMCU_CLK_ENABLE( );

    HAL_DBGMCU_EnableDBGSleepMode( );
    HAL_DBGMCU_EnableDBGStopMode( );
    HAL_DBGMCU_EnableDBGStandbyMode( );
#else
    __HAL_RCC_DBGMCU_CLK_ENABLE( );
    HAL_DBGMCU_DisableDBGSleepMode( );
    HAL_DBGMCU_DisableDBGStopMode( );
    HAL_DBGMCU_DisableDBGStandbyMode( );
    __HAL_RCC_DBGMCU_CLK_DISABLE( );
#endif
}

void stm32_common_mcu_reinit_after_longsleep() {
  DPRINT("stm32_common_mcu_reinit_after_sleep");
  
  /* Set voltage scaling to enable 32 32MHz */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOS) != RESET) {};

  /* Enable HSI */
  assert(HAL_RCC_OscConfig(&RCC_OSC_active_hsi) == HAL_OK);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2  clocks dividers */
  assert(HAL_RCC_ClockConfig(&RCC_ClkInitStruct_active, FLASH_LATENCY_1) == HAL_OK);

  /* DISABLE MSI */
  assert(HAL_RCC_OscConfig(&RCC_OSC_active_msi) == HAL_OK);
}

void stm32_common_mcu_reinit_after_shortsleep() {
  DPRINT("stm32_common_mcu_reinit_after_shortsleep");

  /* Enable HSI */
  //assert(HAL_RCC_OscConfig(&RCC_OSC_active_hsi) == HAL_OK);

  init_clock();

}

void stm32_common_mcu_prepare_longsleep() {
  DPRINT("stm32_common_mcu_prepare_sleep");

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();

// Enable MSI
  assert(HAL_RCC_OscConfig(&RCC_OSC_sleep_msi) == HAL_OK);

  //Select MSI as system clock   
  assert(HAL_RCC_ClockConfig(&RCC_ClkInitStruct_sleep, FLASH_LATENCY_0) == HAL_OK);

  // Dissable HSI
  assert(HAL_RCC_OscConfig(&RCC_OSC_sleep_hsi) == HAL_OK);

  // dynamic voltage scaling
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOS) != RESET) {};
}

void stm32_common_mcu_prepare_shortsleep() {
  DPRINT("stm32_common_mcu_prepare_shortsleep");
}

void stm32_common_mcu_init()
{
  HAL_Init();
  init_clock();
  hw_system_save_reboot_reason();
}

uint32_t HAL_GetTick(void)
{
	return timer_get_counter_value();
}

