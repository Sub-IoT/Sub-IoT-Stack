/**************************************************************************//**
 * @file
 * @brief Simple LED Blink Demo for EFM32GG_STK3700
 * @version 3.20.5
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
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"

volatile uint32_t msTicks; /* counts 1ms timeTicks */

void Delay(uint32_t dlyTicks);

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       /* increment counter necessary in Delay()*/
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) ;
}

typedef struct
{
  GPIO_Port_TypeDef   port;
  unsigned int        pin;
} leds_t;

static const leds_t leds[ 2 ] = {{gpioPortE,2},{gpioPortE,3}};

void led_init(void)
{
  int i;

  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  for ( i=0; i< 2; i++ )
  {
    GPIO_PinModeSet(leds[i].port, leds[i].pin, gpioModePushPull, 0);
  }
}

void led_set(int ledNo)
{
	GPIO_PinOutSet(leds[ledNo].port, leds[ledNo].pin);
}

void led_toggle(int ledNo)
{
    GPIO_PinOutToggle(leds[ledNo].port, leds[ledNo].pin);
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();

  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

  /* Initialize LED driver */
  led_init();
  led_set(0);

  /* Infinite blink loop */
  while (1)
  {
    led_toggle(0);
    led_toggle(1);
    Delay(1000);
  }
}
