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

#include "platform.h"
#include "stm32_device.h"
#include "bootstrap.h"
#include "hwgpio.h"
#include "hwsystem.h"
#include "debug.h"
#include "stm32_common_gpio.h"
#include "hwradio.h"
#include "errors.h"
#include "platform_defs.h"

//#define DPRINT(...)
#define DPRINT(...) log_print_string(__VA_ARGS__)

static bool modem_listen_uart_inited = false;

#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
static void app_uart_on() {
  uart_init(PLATFORM_CONSOLE_UART, PLATFORM_CONSOLE_BAUDRATE, PLATFORM_CONSOLE_LOCATION);
  console_enable();
  console_rx_interrupt_enable();
}

static void app_uart_off() {
  console_disable();
}

static void modem_listen(void* arg) {
  if(!modem_listen_uart_inited) {
    modem_listen_uart_inited = true;
    app_uart_on();
  }

  if(hw_gpio_get_in(MCU2MODEM_INT_PIN)) {
    // prevent the MCU to go back to stop mode by scheduling ourself again until pin goes low,
    // to keep UART RX enabled
    sched_post_task_prio(&modem_listen, MIN_PRIORITY, NULL);
  } else {
    DPRINT("!!! modem released\n");
    app_uart_off();
  }
}

// TODO move to modem module
static void on_modem_wakeup(pin_id_t pin_id, uint8_t event_mask)
{
  hw_gpio_set(MODEM2MCU_INT_PIN); // TODO tmp
  if(event_mask & GPIO_RISING_EDGE) {
    DPRINT("!!! modem wakeup requested");

    // delay uart init until scheduled task, MCU clock will only be initialzed correclty after ISR, when entering scheduler again
    modem_listen_uart_inited = false;
    sched_post_task(&modem_listen);
  }
}

// TODO move to modem module
void platform_app_mcu_wakeup() {
  log_print_string("!!! wake app mcu @ %d", timer_get_counter_value()); // TODO tmp
  hw_gpio_set(MODEM2MCU_INT_PIN);
  hw_busy_wait(5000); // TODO
  app_uart_on();
}

// TODO move to modem module
void platform_app_mcu_release() {
  log_print_string("!!! release app mcu @ %d", timer_get_counter_value()); // TODO tmp
//  hw_busy_wait(5000); // TODO
  hw_gpio_clr(MODEM2MCU_INT_PIN);
  app_uart_off();
}

#endif

#if defined(USE_SX127X) && defined(PLATFORM_SX127X_USE_RESET_PIN)
// override the weak definition
// TODO might be moved to radio driver is hw_gpio_configure_pin() is part of public HAL API instead of platform specific
void hw_radio_reset()
{
  error_t e;
  e = hw_gpio_configure_pin(SX127x_RESET_PIN, false, GPIO_MODE_OUTPUT_PP, 0); assert(e == SUCCESS);
  hw_busy_wait(150);
  e = hw_gpio_configure_pin(SX127x_RESET_PIN, false, GPIO_MODE_INPUT, 1); assert(e == SUCCESS);
  hw_busy_wait(10000);
}
#endif

void __platform_init()
{
    stm32_common_mcu_init();
    __gpio_init();

#ifdef USE_SX127X
    hw_radio_io_init();

  #ifdef PLATFORM_SX127X_USE_RESET_PIN
    hw_radio_reset();
  #endif
#endif

    HAL_EnableDBGSleepMode(); // TODO impact on power?
}

void __platform_post_framework_init()
{
  // TODO move to modem module
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
    // define interrupt pins between modem and application mcu
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    hw_gpio_configure_pin_stm(MODEM2MCU_INT_PIN, &GPIO_InitStruct);
    hw_gpio_clr(MODEM2MCU_INT_PIN);

    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    hw_gpio_configure_pin_stm(MCU2MODEM_INT_PIN, &GPIO_InitStruct);
    error_t err = hw_gpio_configure_interrupt(MCU2MODEM_INT_PIN, &on_modem_wakeup, GPIO_RISING_EDGE); assert(err == SUCCESS);
    err = hw_gpio_enable_interrupt(MCU2MODEM_INT_PIN); assert(err == SUCCESS);

    sched_register_task(&modem_listen);
#endif
}

int main()
{
    //initialise the platform itself
    __platform_init();
    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
    //initialise platform functionality that depends on the framework
    __platform_post_framework_init();

    scheduler_run();
    return 0;
}

#if defined(USE_SX127X)
// override the weak definition
void hw_radio_io_init() {
  // configure the radio GPIO pins here, since hw_gpio_configure_pin() is MCU
  // specific and not part of the common HAL API
  hw_gpio_configure_pin(SX127x_DIO0_PIN, true, GPIO_MODE_INPUT, 0);
  hw_gpio_disable_interrupt(SX127x_DIO0_PIN);
  hw_gpio_configure_pin(SX127x_DIO1_PIN, true, GPIO_MODE_INPUT, 0);
  hw_gpio_disable_interrupt(SX127x_DIO1_PIN);

  // Antenna switching uses 3 pins on murata ABZ module
  hw_gpio_configure_pin(ABZ_ANT_SW_RX_PIN, false, GPIO_MODE_OUTPUT_PP, 0);
  hw_gpio_configure_pin(ABZ_ANT_SW_TX_PIN, false, GPIO_MODE_OUTPUT_PP, 0);
  hw_gpio_configure_pin(ABZ_ANT_SW_PA_BOOST_PIN, false, GPIO_MODE_OUTPUT_PP, 0);

#ifdef PLATFORM_SX127X_USE_DIO3_PIN
  hw_gpio_configure_pin(SX127x_DIO3_PIN, true, GPIO_MODE_INPUT, 0);
  hw_gpio_disable_interrupt(SX127x_DIO3_PIN);
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
