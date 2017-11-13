
 /******************************************************************************
  * @file    stm32l0xx_hal_msp.c
  * @author  MCD Application Team
  * @version V1.1.1
  * @date    01-June-2017
  * @brief   msp file for HAL
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_flash.h"
#include "stm32l0xx_hal_flash_ex.h"
#include "debug.h"

/* when fast wake up is enabled, the mcu wakes up in ~20us  * and 
 * does not wait for the VREFINT to be settled. THis is ok for 
 * most of the case except when adc must be used in this case before 
 *starting the adc, you must make sure VREFINT is settled*/
#define ENABLE_FAST_WAKEUP

/**
  * @brief This function configures the source of the time base.
  * @brief  don't enable systick
  * @param TickPriority: Tick interrupt priority.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
   /* Return function status */
  return HAL_OK;
}

/**
  * @brief This function provides delay (in ms)
  * @param Delay: specifies the delay time length, in milliseconds.
  * @retval None
  */
//void HAL_Delay(__IO uint32_t Delay)
//{
//  DelayMs( Delay ); /* based on RTC */
//}

/**
  * @brief  Initializes the MSP.
  * @retval None
  */
void HAL_MspInit(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* Disable the Power Voltage Detector */
  HAL_PWR_DisablePVD( ); 

  /* Enables the Ultra Low Power mode */
  HAL_PWREx_EnableUltraLowPower( );
  
  __HAL_FLASH_SLEEP_POWERDOWN_ENABLE();
  
  /*In debug mode, e.g. when DBGMCU is activated, Arm core has always clocks
   * And will not wait that the FLACH is ready to be read. It can miss in this 
   * case the first instruction. To overcome this issue, the flash remain clcoked during sleep mode
   */
  // TODO DBG( __HAL_FLASH_SLEEP_POWERDOWN_DISABLE(); );
  
#ifdef ENABLE_FAST_WAKEUP
  /*Enable fast wakeUp*/  
  HAL_PWREx_EnableFastWakeUp( );
#else  
  HAL_PWREx_DisableFastWakeUp( );
#endif
}

/**
  * @brief RTC MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param hrtc: RTC handle pointer
  * @note  Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to select 
  *        the RTC clock source; in this case the Backup domain will be reset in  
  *        order to modify the RTC Clock source, as consequence RTC registers (including 
  *        the backup registers) and RCC_CSR register are set to their reset values.  
  * @retval None
  */
//void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
//{
//  RCC_OscInitTypeDef RCC_OscInitStruct;
//  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

//  /*##-1- Configue the RTC clock soucre ######################################*/
//  /* -a- Enable LSE Oscillator */
//  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
//  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
//  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//  {
//    Error_Handler();
//  }

//  /* -b- Select LSI as RTC clock source */
//  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
//  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
//  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
//  {
//    Error_Handler();
//  }

//  /*##-2- Enable the RTC peripheral Clock ####################################*/
//  /* Enable RTC Clock */
//  __HAL_RCC_RTC_ENABLE();
  
//  /*##-3- Configure the NVIC for RTC Alarm ###################################*/
//  HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0x0, 0);
//  HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
//}

///**
//  * @brief RTC MSP De-Initialization
//  *        This function freeze the hardware resources used in this example:
//  *          - Disable the Peripheral's clock
//  * @param hrtc: RTC handle pointer
//  * @retval None
//  */
//void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
//{
//  /* Reset peripherals */
//  __HAL_RCC_RTC_DISABLE();
//}


///**
//  * @brief  Alarm A callback.
//  * @param  hrtc: RTC handle
//  * @retval None
//  */
//void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
//{
//  TimerIrqHandler( );
//}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected to the EXTI line.
  * @retval None
  */
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//  HW_GPIO_IrqHandler( GPIO_Pin );
//}

/**
  * @brief  Gets IRQ number as a function of the GPIO_Pin.
  * @param  GPIO_Pin: Specifies the pins connected to the EXTI line.
  * @retval IRQ number
  */
IRQn_Type MSP_GetIRQn( uint16_t GPIO_Pin)
{
  switch( GPIO_Pin )
  {
    case GPIO_PIN_0:  
    case GPIO_PIN_1:  return EXTI0_1_IRQn;
    case GPIO_PIN_2: 
    case GPIO_PIN_3:  return EXTI2_3_IRQn;
    case GPIO_PIN_4:  
    case GPIO_PIN_5:  
    case GPIO_PIN_6:
    case GPIO_PIN_7:
    case GPIO_PIN_8:
    case GPIO_PIN_9:  
    case GPIO_PIN_10:
    case GPIO_PIN_11:
    case GPIO_PIN_12:
    case GPIO_PIN_13:
    case GPIO_PIN_14:
    case GPIO_PIN_15: 
    default: return EXTI4_15_IRQn;
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
