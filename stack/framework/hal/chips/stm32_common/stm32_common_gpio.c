/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file stm32_common_gpio.c

 *
 */

#include "debug.h"
#include "stm32_device.h"
#include "hwgpio.h"
#include "stm32_common_gpio.h"
#include "stm32_common_mcu.h"
#include "hwatomic.h"
#include "errors.h"
#include "framework_defs.h"

#define PORT_BASE(pin)  ((GPIO_TypeDef*)(pin & ~0x0F))
#define EXTI_LINES_COUNT 16

#define RCC_GPIO_CLK_ENABLE(__GPIO_PORT__)                    \
do {                                                          \
    switch( __GPIO_PORT__)                                    \
    {                                                         \
      case GPIOA_BASE: __HAL_RCC_GPIOA_CLK_ENABLE(); break;   \
      case GPIOB_BASE: __HAL_RCC_GPIOB_CLK_ENABLE(); break;   \
      case GPIOC_BASE: __HAL_RCC_GPIOC_CLK_ENABLE(); break;   \
      case GPIOD_BASE: __HAL_RCC_GPIOD_CLK_ENABLE(); break;   \
      case GPIOE_BASE: __HAL_RCC_GPIOE_CLK_ENABLE(); break;   \
      case GPIOH_BASE: __HAL_RCC_GPIOH_CLK_ENABLE(); break;   \
    }                                                         \
  } while(0)

#define RCC_GPIO_CLK_DISABLE(__GPIO_PORT__)                   \
do {                                                          \
    switch( __GPIO_PORT__)                                    \
    {                                                         \
      case GPIOA_BASE: __HAL_RCC_GPIOA_CLK_DISABLE(); break;  \
      case GPIOB_BASE: __HAL_RCC_GPIOB_CLK_DISABLE(); break;  \
      case GPIOC_BASE: __HAL_RCC_GPIOC_CLK_DISABLE(); break;  \
      case GPIOD_BASE: __HAL_RCC_GPIOD_CLK_DISABLE(); break;  \
      case GPIOE_BASE: __HAL_RCC_GPIOE_CLK_DISABLE(); break;  \
      case GPIOH_BASE: __HAL_RCC_GPIOH_CLK_DISABLE(); break;  \
    }                                                         \
  } while(0)

typedef struct
{
  gpio_isr_ctx_t isr_ctx;
  uint32_t interrupt_port;
} gpio_interrupt_t;


//the list of configured interrupts
static gpio_interrupt_t interrupts[16];

static uint16_t gpio_pins_configured[6];

__LINK_C void __gpio_init()
{
  for(int i = 0; i < 16; i++)
  {
    interrupts[i].isr_ctx.cb = 0x0;
    interrupts[i].isr_ctx.arg = NULL;
    interrupts[i].interrupt_port = 0xFF; //signal that a port has not yet been chosen
  }
  for(int i = 0; i < 6; i++)
    gpio_pins_configured[i] = 0;

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  // set all pins to analog by default
  GPIO_InitTypeDef GPIO_InitStruct= { 0 };
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  uint32_t porta_pins = GPIO_PIN_All;
#ifdef FRAMEWORK_DEBUG_ENABLE_SWD
  // in debug mode keep SWCLK and SWD pin in the default config (AF)
  porta_pins = GPIO_PIN_All & (~( GPIO_PIN_13 | GPIO_PIN_14) );
#endif
  GPIO_InitStruct.Pin = porta_pins;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_All;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();
  __HAL_RCC_GPIOH_CLK_DISABLE();
}

__LINK_C error_t hw_gpio_configure_pin_stm(pin_id_t pin_id, GPIO_InitTypeDef* init_options)
{
  RCC_GPIO_CLK_ENABLE((uint32_t)PORT_BASE(pin_id));
  init_options->Pin = 1 << GPIO_PIN(pin_id);
  if(interrupts[GPIO_PIN(pin_id)].interrupt_port != GPIO_PORT(pin_id) && (init_options->Mode == GPIO_MODE_IT_RISING || init_options->Mode == GPIO_MODE_IT_FALLING || init_options->Mode == GPIO_MODE_IT_RISING_FALLING))
  {
    assert(!LL_EXTI_IsEnabledIT_0_31(init_options->Pin));
    assert(interrupts[GPIO_PIN(pin_id)].interrupt_port == 0xFF);
  }
  start_atomic();
  HAL_GPIO_Init(PORT_BASE(pin_id), init_options);

  if  (init_options->Mode == GPIO_MODE_IT_RISING || init_options->Mode == GPIO_MODE_IT_FALLING || init_options->Mode == GPIO_MODE_IT_RISING_FALLING)
  {
    interrupts[GPIO_PIN(pin_id)].interrupt_port = GPIO_PORT(pin_id);
    // AL-2306 be sure that GPIO interrupt is not yet enabled
    hw_gpio_disable_interrupt(pin_id);
  }
  else if(interrupts[GPIO_PIN(pin_id)].interrupt_port == GPIO_PORT(pin_id))
  {
    // Pin was previously configured as interrupt but now not anymore
    interrupts[GPIO_PIN(pin_id)].interrupt_port = 0xFF;
  }
  end_atomic();

  //RCC_GPIO_CLK_DISABLE((uint32_t)PORT_BASE(pin_id));
  // TODO for now keep the clock for all configured ports as enabled. We might disable them if the pin configuration allows this
  // and if no other pins on the port require this (for eg pins using AF for SPI etc)

  return SUCCESS;
}

__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed, uint32_t mode, unsigned int out)
{
  //do early-stop error checking
  //if((gpio_pins_configured[GPIO_PORT(pin_id)] & (1<<GPIO_PIN(pin_id))))
  //return EALREADY;
  //else if(int_allowed && (interrupts[GPIO_PIN(pin_id)].interrupt_port != 0xFF))
  //return EBUSY;

  //set the pin to be configured
  //gpio_pins_configured[GPIO_PORT(pin_id)] |= (1<<GPIO_PIN(pin_id));

  //configure the pin itself
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode = mode;
  if(GPIO_InitStruct.Mode == GPIO_MODE_IT_RISING
     || GPIO_InitStruct.Mode == GPIO_MODE_IT_FALLING
     || GPIO_InitStruct.Mode == GPIO_MODE_IT_RISING_FALLING)
  {
    GPIO_InitStruct.Pull = GPIO_NOPULL;
  } else {
    if(out) {
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      PORT_BASE(pin_id)->BSRR = 1 << GPIO_PIN(pin_id); // make sure pin level is high when configured (HAL_GPIO_Init() does not take this into account)
    } else {
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    }

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  }

  return hw_gpio_configure_pin_stm(pin_id, &GPIO_InitStruct);
}

__LINK_C error_t hw_gpio_set(pin_id_t pin_id)
{
  HAL_GPIO_WritePin(PORT_BASE(pin_id), 1 << GPIO_PIN(pin_id), GPIO_PIN_SET);
  return SUCCESS;
}

__LINK_C error_t hw_gpio_clr(pin_id_t pin_id)
{
  HAL_GPIO_WritePin(PORT_BASE(pin_id), 1 << GPIO_PIN(pin_id), GPIO_PIN_RESET);
  return SUCCESS;
}

__LINK_C error_t hw_gpio_toggle(pin_id_t pin_id)
{
  HAL_GPIO_TogglePin(PORT_BASE(pin_id), 1 << GPIO_PIN(pin_id));
  return SUCCESS;
}

__LINK_C bool hw_gpio_get_out(pin_id_t pin_id)
{
  // todo check pin is not input pin
  return (HAL_GPIO_ReadPin(PORT_BASE(pin_id), 1 << GPIO_PIN(pin_id)) == GPIO_PIN_SET);
}

__LINK_C bool hw_gpio_get_in(pin_id_t pin_id)
{
  return (HAL_GPIO_ReadPin(PORT_BASE(pin_id), 1 << GPIO_PIN(pin_id)) == GPIO_PIN_SET);
}

static void gpio_int_callback(uint8_t pin)
{
  assert(interrupts[pin].isr_ctx.cb != NULL);
  // when interrupting on both edges and when using low power mode where GPIO clocks are disabled we don't know which edge triggered the interrupt.
  // We could enable the clocks to read in the current GPIO level but most drivers and apps do not need to know this or can determine this based on state.
  // If the upper layer needs to know it can read the GPIO level, but not in the interrupt context (the callback). Scheduling a task in the callback and reading the pin
  // in this taks in thread context ensures that the clocks are active and the actual current state (instead of the last latched) is read.
  // For this reason we pass 0 to the event_mask param of the callback.
  // TODO when only one interrupt edge is configured we _can_ reliably determine the edge so this can be improved
  interrupts[pin].isr_ctx.cb(interrupts[pin].isr_ctx.arg);
}

__LINK_C error_t hw_gpio_set_edge_interrupt(pin_id_t pin_id, uint8_t edge)
{
  uint32_t exti_line = 1 << GPIO_PIN(pin_id);
  switch(edge)
  {
    case GPIO_RISING_EDGE:
      LL_EXTI_DisableFallingTrig_0_31(exti_line);
      LL_EXTI_EnableRisingTrig_0_31(exti_line);
      break;
    case GPIO_FALLING_EDGE:
      LL_EXTI_DisableRisingTrig_0_31(exti_line);
      LL_EXTI_EnableFallingTrig_0_31(exti_line);
      break;
    case (GPIO_RISING_EDGE | GPIO_FALLING_EDGE):
      LL_EXTI_EnableRisingTrig_0_31(exti_line);
      LL_EXTI_EnableFallingTrig_0_31(exti_line);
      break;
    default:
      assert(false);
      break;
  }

  return SUCCESS;
}

__LINK_C error_t hw_gpio_configure_interrupt(pin_id_t pin_id, uint8_t event_mask, gpio_cb_t callback, void *arg)
{
  if (interrupts[GPIO_PIN(pin_id)].interrupt_port != 0xFF)
  {
    if (interrupts[GPIO_PIN(pin_id)].interrupt_port != GPIO_PORT(pin_id))
      return EOFF;
  } else {
    interrupts[GPIO_PIN(pin_id)].interrupt_port = GPIO_PORT(pin_id);
  }

  if(callback == 0x0 || event_mask > (GPIO_RISING_EDGE | GPIO_FALLING_EDGE))
    return EINVAL;

  error_t err;
  start_atomic();
  //do this check atomically: interrupts[..] callback is altered by this function
  //so the check belongs in the critical section as well
  /*if(interrupts[GPIO_PIN(pin_id)].callback != 0x0 && interrupts[GPIO_PIN(pin_id)].callback != callback)
    err = EBUSY;
  else
  {*/
    interrupts[GPIO_PIN(pin_id)].isr_ctx.cb = callback;
    interrupts[GPIO_PIN(pin_id)].isr_ctx.arg = arg;

    __HAL_RCC_SYSCFG_CLK_ENABLE();

    // set external interrupt configuration
    uint32_t temp = SYSCFG->EXTICR[GPIO_PIN(pin_id) >> 2U];
    CLEAR_BIT(temp, ((uint32_t)0x0FU) << (4U * (GPIO_PIN(pin_id) & 0x03U)));
    SET_BIT(temp, (GPIO_PORT_MASK(pin_id)) << (4 * (GPIO_PIN(pin_id) & 0x03U)));
    SYSCFG->EXTICR[GPIO_PIN(pin_id) >> 2U] = temp;


    uint32_t exti_line = 1 << GPIO_PIN(pin_id);
    /* First Disable Event on provided Lines */
    LL_EXTI_DisableEvent_0_31(exti_line);
    /* Then Enable IT on provided Lines */
    LL_EXTI_EnableIT_0_31(exti_line);

    switch(event_mask)
    {
      case GPIO_RISING_EDGE:
        LL_EXTI_DisableFallingTrig_0_31(exti_line);
        LL_EXTI_EnableRisingTrig_0_31(exti_line);
        break;
      case GPIO_FALLING_EDGE:
        LL_EXTI_DisableRisingTrig_0_31(exti_line);
        LL_EXTI_EnableFallingTrig_0_31(exti_line);
        break;
      case (GPIO_RISING_EDGE | GPIO_FALLING_EDGE):
        LL_EXTI_EnableRisingTrig_0_31(exti_line);
        LL_EXTI_EnableFallingTrig_0_31(exti_line);
        break;
      case 0:
        LL_EXTI_DisableIT_0_31(exti_line);
        break;
      default:
        assert(false);
        break;
    }
    err = SUCCESS;
  //}

  __HAL_RCC_SYSCFG_CLK_DISABLE();

  end_atomic();
  return err;
}


__LINK_C error_t hw_gpio_enable_interrupt(pin_id_t pin_id)
{
  __HAL_GPIO_EXTI_CLEAR_IT(1 << GPIO_PIN(pin_id));

  uint32_t exti_line = 1 << GPIO_PIN(pin_id);
  LL_EXTI_EnableIT_0_31(exti_line);

#if defined(STM32L0)
  if(GPIO_PIN(pin_id) <= 1) {
    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 2, 0); // TODO on boot
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
    return SUCCESS;
  } else if (GPIO_PIN(pin_id) > 1 && GPIO_PIN(pin_id) <= 3) {
    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 2, 0); // TODO on boot
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
    return SUCCESS;
  } else if (GPIO_PIN(pin_id) > 3 && GPIO_PIN(pin_id) <= 15) {
    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 2, 0); // TODO on boot
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
    return SUCCESS;
  } else {
    assert(false);
    return FAIL;
  }
}
#elif defined(STM32L1)
  if(GPIO_PIN(pin_id) <= 4) {
    HAL_NVIC_SetPriority(EXTI0_IRQn + GPIO_PIN(pin_id), 2, 0); // TODO on boot
    HAL_NVIC_EnableIRQ(EXTI0_IRQn + GPIO_PIN(pin_id));
    return SUCCESS;
  } else if (GPIO_PIN(pin_id) <= 9) {
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0); // TODO on boot
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    return SUCCESS;
  } else if (GPIO_PIN(pin_id) > 9 && GPIO_PIN(pin_id) <= 15) {
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0); // TODO on boot
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    return SUCCESS;
  } else {
    assert(false);
    return FAIL;
  }
}
#else
  #error "STM32 family not supported"
#endif

__LINK_C error_t hw_gpio_disable_interrupt(pin_id_t pin_id)
{
  //TODO: check if no other pins are still using the interrupt
  /*
   if(GPIO_PIN(pin_id) <= 1) {
    HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
    return SUCCESS;
  } else if (GPIO_PIN(pin_id) >= 2 && GPIO_PIN(pin_id) < 4) {
    HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
    return SUCCESS;
  } else if (GPIO_PIN(pin_id) >= 4 && GPIO_PIN(pin_id) < 16) {
    HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
    return SUCCESS;
  }
  */

	uint32_t exti_line = 1 << GPIO_PIN(pin_id);
	LL_EXTI_DisableIT_0_31(exti_line);

	return SUCCESS;
}

void EXTI_IRQHandler()
{
  uint32_t exti_interrrupts = EXTI->PR & EXTI->IMR;
  for (uint8_t pin_nr = 0; pin_nr < EXTI_LINES_COUNT; pin_nr++)
  {
    uint16_t pin = 1 << pin_nr;
    if(pin & exti_interrrupts)
    {
      if(__HAL_GPIO_EXTI_GET_IT(pin) != RESET)
      {
        __HAL_GPIO_EXTI_CLEAR_IT(pin);
        gpio_int_callback(pin_nr);
        return;
      }
    }
  }
}

