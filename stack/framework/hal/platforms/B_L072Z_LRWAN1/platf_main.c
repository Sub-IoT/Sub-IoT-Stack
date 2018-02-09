/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stm32l0xx_mcu.h"
#include "bootstrap.h"
#include "hwgpio.h"
#include "hwleds.h"
#include "led.h"
#include "button.h"
#include "stm32l0xx_gpio.h"
#include "hwsystem.h"
#include "debug.h"
#include "stm32l0xx_hal.h"
#include "hwdebug.h"
#include "hwradio.h"

#if defined(USE_SX127X) && defined(PLATFORM_SX127X_USE_RESET_PIN)
static void reset_sx127x()
{
  error_t e;
  e = hw_gpio_configure_pin(SX127x_RESET_PIN, false, GPIO_MODE_OUTPUT_PP, 0); assert(e == SUCCESS);
  hw_busy_wait(150);
  e = hw_gpio_configure_pin(SX127x_RESET_PIN, false, GPIO_MODE_INPUT, 1); assert(e == SUCCESS);
  hw_busy_wait(10000);
}
#endif

void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* Enable MSI Oscillator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.MSICalibrationValue=0x00;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select MSI as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
 /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /* Disable Power Control clock */
  __HAL_RCC_PWR_CLK_DISABLE();

}


void __platform_init()
{
    SystemClock_Config();
    //__stm32l0xx_mcu_init();
    __gpio_init();
    //__hw_debug_init();
    //hw_radio_io_deinit();
#ifdef USE_SX127X
    hw_radio_io_init();

  #ifdef PLATFORM_SX127X_USE_RESET_PIN
    reset_sx127x();
  #endif
#endif

  HAL_EnableDBGSleepMode(); // TODO impact on power?
}

void __platform_post_framework_init()
{
    __ubutton_init();
    led_init();
}

int main()
{
//    //initialise the platform itself
    __platform_init();
//    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
//    //initialise platform functionality that depends on the framework
//    __platform_post_framework_init();

    scheduler_run();
    return 0;
}

#if defined(USE_SX127X)
// override the weak definition
void hw_radio_io_init() {
  // configure the radio GPIO pins here, since hw_gpio_configure_pin() is MCU
  // specific and not part of the common HAL API
  hw_gpio_configure_pin(SX127x_DIO0_PIN, true, GPIO_MODE_INPUT, 0);
  hw_gpio_configure_pin(SX127x_DIO1_PIN, true, GPIO_MODE_INPUT, 0);

  // Antenna switching uses 3 pins on murata ABZ module
  hw_gpio_configure_pin(ABZ_ANT_SW_RX_PIN, false, GPIO_MODE_OUTPUT_PP, 0);
  hw_gpio_configure_pin(ABZ_ANT_SW_TX_PIN, false, GPIO_MODE_OUTPUT_PP, 0);
  hw_gpio_configure_pin(ABZ_ANT_SW_PA_BOOST_PIN, false, GPIO_MODE_OUTPUT_PP, 0);

#ifdef PLATFORM_SX127X_USE_DIO3_PIN
  hw_gpio_configure_pin(SX127x_DIO3_PIN, true, GPIO_MODE_INPUT, 0);
#endif
#ifdef PLATFORM_SX127X_USE_VCC_TXCO
  hw_gpio_configure_pin(SX127x_VCC_TXCO, false, GPIO_MODE_OUTPUT_PP, 1);
  hw_gpio_set(SX127x_VCC_TXCO);
#endif
}

// override the weak definition
void hw_radio_io_deinit() {
  GPIO_InitTypeDef initStruct={0};
  initStruct.Mode = GPIO_MODE_ANALOG;

  hw_gpio_configure_pin_stm(SX127x_DIO0_PIN, &initStruct);
  hw_gpio_configure_pin_stm(SX127x_DIO1_PIN, &initStruct);
#ifdef PLATFORM_SX127X_USE_DIO3_PIN
  hw_gpio_configure_pin_stm(SX127x_DIO3_PIN, &initStruct);
#endif
  hw_gpio_configure_pin_stm(ABZ_ANT_SW_RX_PIN, &initStruct);
  hw_gpio_clr(ABZ_ANT_SW_RX_PIN);
  hw_gpio_configure_pin_stm(ABZ_ANT_SW_TX_PIN, &initStruct);
  hw_gpio_clr(ABZ_ANT_SW_TX_PIN);
  hw_gpio_configure_pin_stm(ABZ_ANT_SW_PA_BOOST_PIN, &initStruct);
  hw_gpio_clr(ABZ_ANT_SW_PA_BOOST_PIN);
#ifdef PLATFORM_SX127X_USE_RESET_PIN
  hw_gpio_configure_pin_stm(SX127x_RESET_PIN, &initStruct);
#endif
#ifdef PLATFORM_SX127X_USE_VCC_TXCO
  hw_gpio_configure_pin_stm(SX127x_VCC_TXCO, &initStruct);
#endif
// TODO do not deinit SPI for now, may be used for other slave as well
//  hw_gpio_configure_pin_stm(SX127x_SPI_PIN_CS, &initStruct);
//  hw_gpio_configure_pin_stm(PIN(GPIO_PORTA, 6), &initStruct);
//  hw_gpio_configure_pin_stm(PIN(GPIO_PORTA, 7), &initStruct);
//  hw_gpio_configure_pin_stm(PIN(GPIO_PORTB, 3), &initStruct);

  // TODO remove here, ports can be used for other purposes, needs to be managed by gpio driver
//  __HAL_RCC_GPIOA_CLK_DISABLE();
//  __HAL_RCC_GPIOB_CLK_DISABLE();
//  __HAL_RCC_GPIOC_CLK_DISABLE();
//  __HAL_RCC_GPIOD_CLK_DISABLE();
//  __HAL_RCC_GPIOE_CLK_DISABLE();
//  __HAL_RCC_GPIOH_CLK_DISABLE();
}
#endif
