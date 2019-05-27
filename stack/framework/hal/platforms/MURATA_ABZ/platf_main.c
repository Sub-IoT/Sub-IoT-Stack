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

#define DPRINT(...)
//#define DPRINT(...) log_print_string(__VA_ARGS__)

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

    __HAL_RCC_CLEAR_RESET_FLAGS();
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

void hw_deinit_pheriperals()
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  // set PA2 (=MODEM2MCUINT) as analog // TODO move to modem module?
   //GPIOA->MODER |= 0x03 << (2*2);
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
   assert(hw_gpio_configure_pin(MODEM2MCU_INT_PIN, false, GPIO_MODE_OUTPUT_PP, 0) == SUCCESS);
#endif
   //__HAL_RCC_GPIOA_CLK_DISABLE();

  // TODO remove here, ports can be used for other purposes, needs to be managed by gpio driver
//  __HAL_RCC_GPIOB_CLK_DISABLE();
//  __HAL_RCC_GPIOC_CLK_DISABLE();
//  __HAL_RCC_GPIOD_CLK_DISABLE();
//  __HAL_RCC_GPIOE_CLK_DISABLE();
//  __HAL_RCC_GPIOH_CLK_DISABLE();

  DPRINT("EXTI->PR %x", EXTI->PR);
  //assert(EXTI->PR == 0);
  //assert(LPTIM1->ISR == 0);
  //assert(USART2->ISR == 0xc0);
  EXTI->IMR |= 1 << 29; // ensure we can always wake up from LPTIM1 // TODO should be done by driver
//EXTI->IMR = 0x3f840000; // TODO temp
  DPRINT("EXTI->IMR %x", EXTI->IMR);
  //assert(EXTI->IMR == 0x3F840001); // 3f842000 = default value (direct lines) + MCU2MODEM interrupt

  DPRINT("RCC_AHBENR %x", RCC->AHBENR); // 0x100 => MIFEN // TODO see also PWR_CR->DS_EE_KOFF
  DPRINT("RCC_AHBSMENR %x", RCC->AHBSMENR); // 0x1111301 // TODO valid in stop mode?
  //assert(RCC->APB1ENR == 0x80000000); // only LPTIM1 enabled
  DPRINT("RCC_APB1SMENR %x", RCC->APB1SMENR); // TODO valid in stop mode?
  DPRINT("RCC_APB2ENR %x", RCC->APB2ENR); // 0x404000 => USART1 + DBG => 0x00
  // TODO assert(RCC->APB2ENR == 0 || RCC->APB2ENR == 0x400000); // TODO 0x400000 only when debug enabled;
  DPRINT("RCC_APB2SMENR %x", RCC->APB2SMENR); // TODO valid in stop mode?
  DPRINT("RCC->IOPENR %x", RCC->IOPENR);
  RCC->IOPSMENR = 0;
  DPRINT("RCC->IOPSMENR %x", RCC->IOPSMENR);
  //assert(RCC->IOPENR & 1); // PORTA for MCU int // TODO sx1276 IRQs?
  DPRINT("RCC_CCIPR %x", RCC->CCIPR); // c0000 => LSE used for LPTIM
  //assert(RCC->CCIPR == 0xc0000); // LSE used for LPTIM
  DPRINT("RCC->CSR %x\n", RCC->CSR);
  //assert(RCC->CSR == 0x300 || RCC->CSR == 0x50300 ); // LSE  // TODO RTC used for lorwan stack for now. TODO: LSI needed for IWDG?) //TODO LORAWAN asserts here
  DPRINT("PWR->CR %x", PWR->CR); // 0xF00 => ULP+FWU ok, DBP TODO
  DPRINT("PWR->CSR %x", PWR->CSR);
  //assert(PWR->CSR == 0x8); // 0x8 => VREFINTRDYF ok


// TODO
  //#ifndef NDEBUG
//  assert(GPIOA->MODER == 0xEBFFFFFC); // PA13 & PA14 = SWD, PA0 = MCU2MODEM interrupt
//#else
//  assert(GPIOA->MODER == 0xFFFFFFFC);
//#endif

//  uint32_t gpioa_moder_mask = 0;
//  gpioa_moder_mask |= 0b11 << (0 * 2); // MCU2MODEM INT => TODO hardcoded for now
//  gpioa_moder_mask |= 0b11 << (11 * 2); // MODEM2MCU INT => TODO hardcoded for now
//  gpioa_moder_mask |= 0b11 << (1 * 2); // A1 = ANT SW, can be enabled during RX or TX
//#ifdef FRAMEWORK_DEBUG_ENABLE_SWD
//  gpioa_moder_mask |= 0b11 << (13 * 2); // SWD
//  gpioa_moder_mask |= 0b11 << (14 * 2); // SWD
//#endif
//  gpioa_moder_mask |= 0b11 << (6 * 2); // SPI MISO // TODO not always disabled
//  gpioa_moder_mask |= 0b11 << (7 * 2); // SPI MOSI // TODO not always disabled
//  gpioa_moder_mask |= 0b11 << (12 * 2); // TCXO VCC // TODO not always disabled
//  gpioa_moder_mask |= 0b11 << (15 * 2); // SPI CS // TODO not always disabled
//  DPRINT("GPIOA->MODER %x, mask %x, result %x", GPIOA->MODER, gpioa_moder_mask, GPIOA->MODER | gpioa_moder_mask);
//  assert((GPIOA->MODER | gpioa_moder_mask) == 0xFFFFFFFF);


//  uint32_t gpiob_moder_mask = 0;
//  gpiob_moder_mask |= 0b11 << (1 * 2); // B1 = DIO1
//  gpiob_moder_mask |= 0b11 << (4 * 2); // B4 = DIO0
//  gpiob_moder_mask |= 0b11 << (3 * 2); // B3 = SPI CLK // TODO not always disabled (for example during TX)
//  DPRINT("GPIOB->MODER %x, mask %x, result %x", GPIOB->MODER, gpiob_moder_mask, GPIOB->MODER | gpiob_moder_mask);
//  assert((GPIOB->MODER | gpiob_moder_mask) == 0xFFFFFFFF);

//  uint32_t gpioc_moder_mask = 0;
//  gpioc_moder_mask |= 0b11 << (0 * 2); // C0 = nRESET sx127x // TODO
//  gpioc_moder_mask |= 0b11 << (1 * 2); // C1 = ANT SW, can be enabled during RX or TX
//  gpioc_moder_mask |= 0b11 << (2 * 2); // C2 = ANT SW, can be enabled during RX or TX
//  gpioc_moder_mask |= 0b11 << (13 * 2); // DIO3, can be enabled during RX or TX
//  DPRINT("GPIOC->MODER %x, mask %x, result %x", GPIOC->MODER, gpioc_moder_mask, GPIOC->MODER | gpioc_moder_mask);
//  assert((GPIOC->MODER | gpioc_moder_mask) == 0xFFFFFFFF); //TODO LORAWAN asserts here

  // TODO internal voltage ref
  // TODO GPIO direction
  // TODO voltage scaling?
  // TODO PWR->CSR |= (PWR_CSR_EWUP1 | PWR_CSR_EWUP2);
  // TODO watchdog?
}

