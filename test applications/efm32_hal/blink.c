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
#include "leds.h"
#include "system.h"
#include "log.h"

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


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
    system_init();
    char data[12] = "Hello gecko\n";
  /* Initialize LED driver */
  led_init();
  led_on(0);

  /* Infinite blink loop */
  while (1)
  {
    led_toggle(0);
    led_toggle(1);
    //uart_transmit_message(data, 12);
    log_print_string("%s", data);
    Delay(1000);
  }
}
