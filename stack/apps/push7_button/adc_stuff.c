
#include "debug.h"
#include "hwsystem.h"
#include "log.h"
#include "platform.h"
#include "stm32l0xx_hal_adc.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "adc_stuff.h"

ADC_HandleTypeDef hadc;

static void MX_ADC_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = { 0 };
    hadc.Instance = ADC1;
    hadc.Init.OversamplingMode = ADC_OVERSAMPLING_RATIO_256;
    hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.SamplingTime = ADC_SAMPLETIME_160CYCLES_5;
    hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc.Init.LowPowerAutoWait = DISABLE;
    hadc.Init.LowPowerFrequencyMode = DISABLE;
    hadc.Init.LowPowerAutoPowerOff = DISABLE;
    if (HAL_ADC_Init(&hadc) != HAL_OK) {
        log_print_string("error");
    }
    sConfig.Channel = ADC_CHANNEL_5;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
        log_print_string("error");
    }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    if (hadc->Instance == ADC1) {
        __HAL_RCC_ADC1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        __HAL_RCC_ADC1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);
    }
}

void adc_stuff_init()
{
    MX_ADC_Init();
}

uint16_t get_battery_voltage()
{
    float battery_voltage;
    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);

    //Vbat = measured voltage / (R1/R1+R2)
    // measured voltage = (ADC_value / (2^12) ) * VDD
    // Vbat = (ADC_value / (2^12) ) * VDD  * (R1+R2/R1)

    // VDD=2700  -   (R1+R2/R1) = ( (10+6.04) /10)
    // (1/4096)*2700*(16.04/10) = 1.05732421876

    battery_voltage = (float) HAL_ADC_GetValue(&hadc) * 1.05732421876; 

    HAL_ADC_Stop(&hadc);
    return (uint16_t)(round(battery_voltage));
}