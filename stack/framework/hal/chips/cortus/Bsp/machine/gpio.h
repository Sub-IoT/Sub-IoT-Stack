/*********************************************************************************
 *  This confidential and proprietary software may be used only as authorized 
 *                       by a licensing agreement from                           
 *                            Cortus S.A.
 *
 *           (C) Copyright 2004, 2005, Cortus S.A.
 *                           ALL RIGHTS RESERVED
 *
 *  The entire notice above must be reproduced on all authorized copies
 *  and any such reproduction must be pursuant to a licensing agreement 
 *  from Cortus S.A. (http://www.cortus.com)
 *
 * File: gpio.h
 *
 *********************************************************************************/

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
