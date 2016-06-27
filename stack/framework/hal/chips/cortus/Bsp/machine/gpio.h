/*******************************************************************************
* File: gpio.h
* @section License
* <b>(C) Copyright 2005 Cortus S.A, http://www.cortus.com
*******************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Cortus S.A has no
* obligation to support this Software. Cortus S.A is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Cortus S.A will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
******************************************************************************/

#ifndef _GPIO_H
#define _GPIO_H
#include <machine/sfradr.h>

typedef struct GPio
{
    volatile unsigned out;
    volatile unsigned in;
    volatile unsigned dir;
    volatile unsigned old_in;
    volatile unsigned mask;
} GPio;


/** GPIO ports identificator. */
typedef enum
{
  gpioPortA = SFRADR_GPIO1, /**< Port A */
  gpioPortB = SFRADR_GPIO1, /**< Port B */
  //gpioPortC = 2, /**< Port C */
  //gpioPortD = 3, /**< Port D */
  //gpioPortE = 4, /**< Port E */
  //gpioPortF = 5  /**< Port F */
} GPIO_Port_TypeDef;

#if 0
/** GPIO drive mode. */
typedef enum
{
  /** Default 6mA */
  gpioDriveModeStandard = GPIO_P_CTRL_DRIVEMODE_STANDARD,
  /** 0.5 mA */
  gpioDriveModeLowest   = GPIO_P_CTRL_DRIVEMODE_LOWEST,
  /** 20 mA */
  gpioDriveModeHigh     = GPIO_P_CTRL_DRIVEMODE_HIGH,
  /** 2 mA */
  gpioDriveModeLow      = GPIO_P_CTRL_DRIVEMODE_LOW
} GPIO_DriveMode_TypeDef;
#endif

/** Pin mode. For more details on each mode, please refer to the EFM32
 * reference manual. */
typedef enum
{
  /** Input enabled. Filter if DOUT is set */
  gpioModeInput                     = 0,
  /** Push-pull output */
  gpioModePushPull                  = 1

} GPIO_Mode_TypeDef;




#ifdef __APS__
#define gpio ((GPio *)SFRADR_GPIO1)
#else
extern GPio __gpio;
#define gpio (&__gpio)
#endif
#endif
