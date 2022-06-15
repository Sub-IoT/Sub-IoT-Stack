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
#include <debug.h>
#include <stdio.h>

//#include "hwgpio.h"
#include "hwadc.h"

#include "platform.h"
#include "ports.h"
#include "stm32_device.h"
#include "stm32_common_gpio.h"
#include "log.h"



#if defined(STM32L0)


static ADC_HandleTypeDef hadc;

error_t adc_init(ADC_Reference reference, ADC_Input input, uint32_t adc_frequency)
{
  assert(reference==adcReference1V25);
  assert(input == adcInputUnused);
  assert(adc_frequency == 0);
  ADC_ChannelConfTypeDef sConfig = { 0 };
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
   */
  hadc.Instance                   = ADC1;
  hadc.Init.OversamplingMode      = DISABLE;
  hadc.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc.Init.SamplingTime          = ADC_SAMPLETIME_160CYCLES_5;
  hadc.Init.ScanConvMode          = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc.Init.ContinuousConvMode    = ENABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
  hadc.Init.Overrun               = ADC_OVR_DATA_PRESERVED;
  hadc.Init.LowPowerAutoWait      = DISABLE;
  hadc.Init.LowPowerFrequencyMode = ENABLE;
  hadc.Init.LowPowerAutoPowerOff  = DISABLE;
  if(HAL_ADC_Init(&hadc) != HAL_OK)
  {
      log_print_error_string("ADC init failed");
      return FAIL;
  }

    /** Configure for the selected ADC regular channel to be converted.
   */
  for(uint8_t i = 0; i < ADC_CHANNEL_COUNT;  i++)
  {
    if(HAL_ADC_ConfigChannel(&hadc, &adc_channels[i].adc_channel) != HAL_OK)
    {
      log_print_error_string("ADC config channel failed");
      return FAIL;
    }
  }
  return SUCCESS;
}

error_t adc_read_all(uint16_t* measurements)
{
  HAL_ADC_Start(&hadc);
  for(uint8_t i = 0; i < ADC_CHANNEL_COUNT;  i++)
  {
    HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
    measurements[i] = HAL_ADC_GetValue(&hadc);
  }
  HAL_ADC_Stop(&hadc);
}

/**
 * @brief ADC MSP Initialization
 * @param hadc: ADC handle pointer
 * @retval None
 */

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc1)
{
    if(hadc1->Instance == ADC1)
    {
        /* Peripheral clock enable */
        __HAL_RCC_ADC1_CLK_ENABLE();
#if ADC_PINS_COUNT > 0
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        for(uint8_t i = 0; i < ADC_PINS_COUNT;  i++)
        {
          GPIO_InitStruct.Pin = 1 << GPIO_PIN(adc_pins[i]);
          GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
          GPIO_InitStruct.Pull = GPIO_NOPULL;
          HAL_GPIO_Init(PORT_BASE(pin_id), &GPIO_InitStruct);
        }
#endif
    }
}

/**
 * @brief ADC MSP De-Initialization
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc1)
{
    if(hadc1->Instance == ADC1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_ADC1_CLK_DISABLE();

        #if ADC_PINS_COUNT > 0
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        for(uint8_t i = 0; i < ADC_PINS_COUNT;  i++)
        {
          HAL_GPIO_DeInit(PORT_BASE(pin_id), 1 << GPIO_PIN(adc_pins[i]));
        }
#endif
    }
}

#endif