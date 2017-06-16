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

/*! \file stm32l152_mcu.c
 *
 */

#include "stm32l1xx_hal.h"

#include "stm32l1xx_hal_conf.h"
#include "assert.h"

extern void Error_Handler();
void  SystemClock_Config();

#define BUTTON_Pin GPIO_PIN_13
#define BUTTON_GPIO_Port GPIOC

void __stm32l152_mcu_init()
{

	/* MCU Configuration----------------------------------------------------------*/
	//__HAL_RCC_PWR_CLK_ENABLE();
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */

	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	//MX_GPIO_Init();


}

void SystemClock_Config(void)
{

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	/**Configure the main internal regulator output voltage
	*/
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/**Initializes the CPU, AHB and APB busses clocks
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 16;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV4;
  // TODO using RCC_PLL_DIV3 is giving problems for now when resuming from
  // WFI and with a debugger attached; we enter FaultHandler in this case.
  // Lowering the clock frequency solves this for now.

	HAL_StatusTypeDef status = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	assert(status == HAL_OK);


	/**Initializes the CPU, AHB and APB busses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
						  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	status = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);
	assert(status == HAL_OK);

	/**Configure the Systick interrupt time
	*/
//	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
//
//	/**Configure the Systick
//	*/
//	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
//
//	/* SysTick_IRQn interrupt configuration */
//	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

