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
#include "timer.h"

void Timer_Loop(void);

timer_event timer = { .next_event = 1024 , .f = Timer_Loop };

char data[12] = "Hello gecko\n";

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void Timer_Loop(void)
{
    led_toggle(0);
    led_toggle(1);
    log_print_string("%s", data);
    timer_add_event( &timer );
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
    system_init();
    /* Initialize LED driver */
    led_init();
    led_on(0);

    log_print_string("Starting\n");

    timer_init();
    Timer_Loop();

    /* Infinite loop */
    while (1)
    {
    }
}
