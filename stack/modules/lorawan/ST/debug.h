 /******************************************************************************
  * @file    debug.h
  * @author  MCD Application Team
  * @version V1.1.2
  * @date    08-September-2017
  * @brief   Header for driver debug.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/



#include <string.h>
#include <stdio.h>
#include "hw_conf.h"
//#include "vcom.h"
#include "log.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

void DBG_Init( void );

void Error_Handler( void );

#ifdef DEBUG

#define DBG_GPIO_WRITE( gpio, n, x )  HAL_GPIO_WritePin( gpio, n, (GPIO_PinState)(x) )

#define DBG_GPIO_SET( gpio, n )       gpio->BSRR = n

#define DBG_GPIO_RST( gpio, n )       gpio->BRR = n 

#define DBG_RTC_OUTPUT RTC_OUTPUT_DISABLE; /* RTC_OUTPUT_ALARMA on PC13 */

#define DBG( x )  do{ x } while(0)

#ifdef TRACE

#define PRINTF(...)    log_print_string(__VA_ARGS__)

#define DBG_PRINTF(...)    log_print_string(__VA_ARGS__)

#define DBG_PRINTF_CRITICAL(...)   log_print_string(__VA_ARGS__)

#else /*TRACE*/

#define PRINTF(...)

#define DBG_PRINTF(...) 

#define DBG_PRINTF_CRITICAL(...) 

#endif /*TRACE*/


#else /* DEBUG */

#define DBG_GPIO_WRITE( gpio, n, x )

#define DBG_GPIO_SET( gpio, n )

#define DBG_GPIO_RST( gpio, n )

#define DBG( x ) do{  } while(0)

#define DBG_PRINTF(...)

#define PRINTF(...)

#define DBG_PRINTF_CRITICAL(...) 
                      
#define DBG_RTC_OUTPUT RTC_OUTPUT_DISABLE;

#endif /* DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* __DEBUG_H__*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
