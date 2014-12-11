/**
 ******************************************************************************
 * @file    platform_config.h
 * @author  MCD Application Team
 * @version V3.4.0
 * @date    29-June-2012
 * @brief   Evaluation board specific configuration file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H
//#define USB_USE_EXTERNAL_PULLUP

/* Includes ------------------------------------------------------------------*/
#if defined(STM32L1XX_MD) || defined(STM32L1XX_HD)|| defined(STM32L1XX_MD_PLUS)
#include "stm32l1xx.h"
#elif defined (STM32F10X_MD) || defined (STM32F10X_HD) || defined (STM32F10X_XL) || defined (STM32F10X_CL)
#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#endif

#if defined(STM32L1XX_MD) || defined(STM32L1XX_HD)|| defined(STM32L1XX_MD_PLUS)
#if !defined(USB_USE_EXTERNAL_PULLUP)
#define STM32L15_USB_CONNECT                SYSCFG_USBPuCmd(ENABLE)
#define STM32L15_USB_DISCONNECT             SYSCFG_USBPuCmd(DISABLE)

#elif defined(USB_USE_EXTERNAL_PULLUP)
/* PA0 is chosen just as illustrating example, you should modify the defines
 below according to your hardware configuration. */
#define USB_DISCONNECT                      GPIOA
#define USB_DISCONNECT_PIN                  GPIO_Pin_10
#define RCC_AHBPeriph_GPIO_DISCONNECT       RCC_AHBPeriph_GPIOA
#define STM32L15_USB_CONNECT                GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN)
#define STM32L15_USB_DISCONNECT             GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN)
#endif /* USB_USE_EXTERNAL_PULLUP */
#elif defined (STM32F10X_CL)
#define USB_DISCONNECT                      0
#define USB_DISCONNECT_PIN                  0
#define RCC_APB2Periph_GPIO_DISCONNECT      0
#endif

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __PLATFORM_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
