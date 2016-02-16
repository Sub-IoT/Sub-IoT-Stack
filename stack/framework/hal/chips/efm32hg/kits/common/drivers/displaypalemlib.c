/**************************************************************************//**
 * @file displaypalemlib.c
 * @brief Platform Abstraction Layer (PAL) for DISPLAY driver on EMLIB based
 *        platforms.
 * @version 3.20.12
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/



#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "bsp.h"
#include "udelay.h"

/* DISPLAY driver inclustions */
#include "displayconfigall.h"
#include "displaypal.h"

#ifdef INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE
#include "em_rtc.h"
#ifdef INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE_HW_ONLY
#include "em_prs.h"
#endif
#endif

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/*******************************************************************************
 ********************************  STATICS  ************************************
 ******************************************************************************/

#ifdef INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE
#ifndef INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE_HW_ONLY
/* GPIO port and pin used for the PAL_GpioPinAutoToggle function. */
static unsigned int gpioPortNo;
static unsigned int gpioPinNo;
#endif

static void rtcSetup(unsigned int frequency);
#endif

/*******************************************************************************
 **************************     GLOBAL FUNCTIONS      **************************
 ******************************************************************************/


/**************************************************************************//**
 * @brief   Initialize the PAL Timer interface
 *
 * @detail  This function initializes all resources required to support the
 *          PAL Timer interface functions.
 *
 * @return  EMSTATUS code of the operation.
 *****************************************************************************/
EMSTATUS PAL_TimerInit (void)
{
  EMSTATUS status = PAL_EMSTATUS_OK;

  UDELAY_Calibrate();

  return status;
}


/**************************************************************************//**
 * @brief   Shutdown the PAL Timer interface
 *
 * @detail  This function releases/stops all resources used by the
 *          PAL Timer interface functions.
 *
 * @return  EMSTATUS code of the operation.
 *****************************************************************************/
EMSTATUS PAL_TimerShutdown (void)
{
  EMSTATUS status = PAL_EMSTATUS_OK;

  /* Nothing to do since the UDELAY_Delay does not use any resources after
     the UDELAY_Calibrate has been called. The UDELAY_Calibrate uses the
     RTC to calibrate the delay loop, and restores the RTC after use. */

  return status;
}


/**************************************************************************//**
 * @brief   Delay for the specified number of micro seconds.
 *
 * @param[in] usecs   Number of micro seconds to delay.
 *
 * @return  EMSTATUS code of the operation.
 *****************************************************************************/
EMSTATUS PAL_TimerMicroSecondsDelay(unsigned int usecs)
{
  EMSTATUS status = PAL_EMSTATUS_OK;

  UDELAY_Delay(usecs);

  return status;
}


#ifdef PAL_TIMER_REPEAT_FUNCTION
/**************************************************************************//**
 * @brief   Call a callback function at the given frequency.
 *
 * @param[in] pFunction  Pointer to function that should be called at the
 *                       given frequency.
 * @param[in] argument   Argument to be given to the function.
 * @param[in] frequency  Frequency at which to call function at.
 *
 * @return  EMSTATUS code of the operation.
 *****************************************************************************/
EMSTATUS PAL_TimerRepeat (void(*pFunction)(void*),
                            void* argument,
                            unsigned int frequency)
{
  if (0 != PAL_TIMER_REPEAT_FUNCTION(pFunction, argument, frequency))
    return PAL_EMSTATUS_REPEAT_FAILED;
  else
    return EMSTATUS_OK;
}
#endif


/**************************************************************************//**
 * @brief   Initialize the PAL GPIO interface
 *
 * @detail  This function initializes all resources required to support the
 *          PAL GPIO interface functions.
 *
 * @return  EMSTATUS code of the operation.
 *****************************************************************************/
EMSTATUS PAL_GpioInit (void)
{
  EMSTATUS status = PAL_EMSTATUS_OK;

  /* Enable the GPIO clock in order to access the GPIO module. */
  CMU_ClockEnable( cmuClock_GPIO, true );

  return status;
}


/**************************************************************************//**
 * @brief   Shutdown the PAL GPIO interface
 *
 * @detail  This function releases/stops all resources used by the
 *          PAL GPIO interface functions.
 *
 * @return  EMSTATUS code of the operation.
 *****************************************************************************/
EMSTATUS PAL_GpioShutdown (void)
{
  EMSTATUS status = PAL_EMSTATUS_OK;

  /* Enable the GPIO clock in order to access the GPIO module. */
  CMU_ClockEnable( cmuClock_GPIO, false );

  return status;
}


/***************************************************************************//**
 * @brief
 *   Set the mode for a GPIO pin.
 *
 * @param[in] port
 *   The GPIO port to access.
 *
 * @param[in] pin
 *   The pin number in the port.
 *
 * @param[in] mode
 *   The desired pin mode.
 *
 * @param[in] platformSpecific
 *   Platform specific value which may need to be set.
 *   For EFM32:
 *   Value to set for pin in DOUT register. The DOUT setting is important for
 *   even some input mode configurations, determining pull-up/down direction.
 ******************************************************************************/
EMSTATUS PAL_GpioPinModeSet(unsigned int   port,
                            unsigned int   pin,
                            PAL_GpioMode_t mode,
                            unsigned int   platformSpecific)
{
  EMSTATUS status = PAL_EMSTATUS_OK;
  GPIO_Mode_TypeDef   emGpioMode;

  /* Convert PAL pin mode to GPIO_Mode_TypeDef defined in em_gpio.h.  */
  switch (mode)
  {
  case palGpioModePushPull:
    emGpioMode = gpioModePushPull;
    break;
  default:
    return PAL_EMSTATUS_INVALID_PARAM;
  }

  GPIO_PinModeSet((GPIO_Port_TypeDef) port, pin, emGpioMode, platformSpecific);

  return status;
}


/***************************************************************************//**
 * @brief
 *   Set a single pin in GPIO data out register to 1.
 *
 * @note
 *   In order for the setting to take effect on the output pad, the pin must
 *   have been configured properly. If not, it will take effect whenever the
 *   pin has been properly configured.
 *
 * @param[in] port
 *   The GPIO port to access.
 *
 * @param[in] pin
 *   The pin to set.
 ******************************************************************************/
EMSTATUS PAL_GpioPinOutSet(unsigned int port, unsigned int pin)
{
  EMSTATUS status = PAL_EMSTATUS_OK;

  GPIO_PinOutSet((GPIO_Port_TypeDef) port, pin);

  return status;
}


/***************************************************************************//**
 * @brief
 *   Set a single pin in GPIO data out port register to 0.
 *
 * @note
 *   In order for the setting to take effect on the output pad, the pin must
 *   have been configured properly. If not, it will take effect whenever the
 *   pin has been properly configured.
 *
 * @param[in] port
 *   The GPIO port to access.
 *
 * @param[in] pin
 *   The pin to set.
 ******************************************************************************/
EMSTATUS PAL_GpioPinOutClear(unsigned int port, unsigned int pin)
{
  EMSTATUS status = PAL_EMSTATUS_OK;

  GPIO_PinOutClear((GPIO_Port_TypeDef) port, pin);

  return status;
}


/***************************************************************************//**
 * @brief
 *   Toggle a single pin in GPIO port data out register.
 *
 * @note
 *   In order for the setting to take effect on the output pad, the pin must
 *   have been configured properly. If not, it will take effect whenever the
 *   pin has been properly configured.
 *
 * @param[in] port
 *   The GPIO port to access.
 *
 * @param[in] pin
 *   The pin to toggle.
 ******************************************************************************/
EMSTATUS PAL_GpioPinOutToggle(unsigned int port, unsigned int pin)
{
  EMSTATUS status = PAL_EMSTATUS_OK;

  GPIO_PinOutToggle((GPIO_Port_TypeDef) port, pin);

  return status;
}


#ifdef INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE
/**************************************************************************//**
 * @brief   Toggle a GPIO pin automatically at the given frequency.
 *
 * @param[in] gpioPort  GPIO port number of GPIO ping to toggle.
 * @param[in] gpioPin   GPIO pin number.
 *
 * @return  EMSTATUS code of the operation.
 *****************************************************************************/
EMSTATUS PAL_GpioPinAutoToggle (unsigned int gpioPort,
                                unsigned int gpioPin,
                                unsigned int frequency)
{
  EMSTATUS status = EMSTATUS_OK;

#ifdef INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE_HW_ONLY

#if defined( BSP_STK_2010 )
  /* We only support auto HW toggling on GPIO port E pin 10 on Zero STK.  */
  if ( (gpioPortE != gpioPort) || (10 != gpioPin) )

#elif defined( BSP_STK_2011 )
  /* We only support auto HW toggling on GPIO port F pin 3 on Happy STK.  */
  if ( (gpioPortF != gpioPort) || (3 != gpioPin) )

#else
#error "Illegal display auto-toggle setup."
#endif
  {
    status = PAL_EMSTATUS_INVALID_PARAM;
  }
  else
  {
    /* Setup PRS to drive the GPIO port E pin 10 which is connected to the
       display com inversion pin (EXTCOMIN) using the RTC COMP0 signal as
       source. */
    uint32_t     source  = PRS_CH_CTRL_SOURCESEL_RTC;
    uint32_t     signal  = PRS_CH_CTRL_SIGSEL_RTCCOMP0;

    /* Enable PRS clock */
    CMU_ClockEnable(cmuClock_PRS, true);

    /* Set up PRS to trigger from an RTC compare match */
    PRS_SourceAsyncSignalSet(LCD_AUTO_TOGGLE_PRS_CH, source, signal);

    /* This outputs the PRS pulse on PE10 which is the EXTCOMIN pin */
    PRS->ROUTE = ( PRS->ROUTE & ~_PRS_ROUTE_LOCATION_MASK )
                 | LCD_AUTO_TOGGLE_PRS_ROUTE_LOC;
    PRS->ROUTE |= LCD_AUTO_TOGGLE_PRS_ROUTE_PEN;
  }
#else
  /* Store GPIO pin data. */
  gpioPortNo = gpioPort;
  gpioPinNo  = gpioPin;
#endif

  if (EMSTATUS_OK == status)
  {
    /* Setup GPIO pin. */
    GPIO_PinModeSet((GPIO_Port_TypeDef)gpioPort, gpioPin, gpioModePushPull, 0 );

    /* Setup RTC to to toggle PRS or generate interrupts at given frequency. */
    rtcSetup(frequency);
  }

  return status;
}


#ifdef  INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE
#ifndef INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE_HW_ONLY
/**************************************************************************//**
 * @brief   RTC Interrupt handler which toggles GPIO pin.
 *
 * @return  N/A
 *****************************************************************************/
void RTC_IRQHandler(void)
{
  /* Clear interrupt source */
  RTC_IntClear(RTC_IF_COMP0);

  /* Toggle GPIO pin. */
  GPIO_PinOutToggle((GPIO_Port_TypeDef)gpioPortNo, gpioPinNo );
}
#endif
#endif


/**************************************************************************//**
 * @brief Enables LFACLK and selects LFXO as clock source for RTC
 *        Sets up the RTC to generate an interrupt every second.
 *****************************************************************************/
static void rtcSetup(unsigned int frequency)
{
  RTC_Init_TypeDef rtcInit = RTC_INIT_DEFAULT;

  /* Enable LE domain registers */
  if ( !( CMU->HFCORECLKEN0 & CMU_HFCORECLKEN0_LE) )
  {
    CMU_ClockEnable(cmuClock_CORELE, true);
  }

#ifdef PAL_RTC_CLOCK_LFXO
  /* LFA with LFXO setup is relatively time consuming. Therefore, check if it
     already enabled before calling. */
  if ( !(CMU->STATUS & CMU_STATUS_LFXOENS) )
  {
    CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
  }
  if ( cmuSelect_LFXO != CMU_ClockSelectGet(cmuClock_LFA) )
  {
    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  }
#elif defined PAL_RTC_CLOCK_LFRCO
  /* Enable LFACLK in CMU (will also enable LFRCO oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);
#elif defined PAL_RTC_CLOCK_ULFRCO
  /* Enable LFACLK in CMU (will also enable ULFRCO oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
#else
#error No clock source for RTC defined.
#endif

  /* Set the prescaler. */
  CMU_ClockDivSet( cmuClock_RTC, cmuClkDiv_2 );

  /* Enable RTC clock */
  CMU_ClockEnable(cmuClock_RTC, true);

  /* Initialize RTC */
  rtcInit.enable   = false;  /* Do not start RTC after initialization is complete. */
  rtcInit.debugRun = false;  /* Halt RTC when debugging. */
  rtcInit.comp0Top = true;   /* Wrap around on COMP0 match. */
  RTC_Init(&rtcInit);

  /* Interrupt at given frequency. */
  RTC_CompareSet(0, (CMU_ClockFreqGet(cmuClock_RTC) / frequency) - 1 );

#ifndef INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE_HW_ONLY
  /* Enable interrupt */
  NVIC_EnableIRQ(RTC_IRQn);
  RTC_IntEnable(RTC_IEN_COMP0);
#endif

  /* Start Counter */
  RTC_Enable(true);
}
#endif  /* INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE */

/** @endcond */
