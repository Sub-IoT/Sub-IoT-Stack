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


#include "bootstrap.h"
#include "hwgpio.h"
#include "hwleds.h"
#include "led.h"
#include "button.h"
#include "stm32_device.h"
#include "platform.h"
#include "debug.h"
#include "hwdebug.h"
#include "errors.h"
#include "stm32_common_gpio.h"

// defined in linker script
extern uint32_t __d7ap_fs_metadata_start;
extern uint32_t __d7ap_fs_metadata_end;
extern uint32_t __d7ap_fs_permanent_files_start;
extern uint32_t __d7ap_fs_permanent_files_end;

static blockdevice_stm32_eeprom_t metadata_bd;
static blockdevice_stm32_eeprom_t permanent_files_bd;

extern uint8_t d7ap_volatile_files_data[FRAMEWORK_FS_VOLATILE_STORAGE_SIZE];
static blockdevice_ram_t ram_bd = (blockdevice_ram_t){
    .base.driver = &blockdevice_driver_ram,
    .base.size = FRAMEWORK_FS_VOLATILE_STORAGE_SIZE,
    .buffer = d7ap_volatile_files_data
};

blockdevice_t * const metadata_blockdevice = (blockdevice_t* const) &metadata_bd;
blockdevice_t * const persistent_files_blockdevice = (blockdevice_t* const) &permanent_files_bd;
blockdevice_t * const volatile_blockdevice = (blockdevice_t* const) &ram_bd;



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

#if defined(USE_SX127X) || defined(USE_NETDEV_DRIVER)
// override the weak definition
void hw_radio_io_init(bool disable_interrupts) {
  // configure the radio GPIO pins here, since hw_gpio_configure_pin() is MCU
  // specific and not part of the common HAL API
  hw_gpio_configure_pin(SX127x_DIO0_PIN, true, GPIO_MODE_INPUT, 0);
  hw_gpio_configure_pin(SX127x_DIO1_PIN, true, GPIO_MODE_INPUT, 0);

  if(disable_interrupts)
  {
      hw_gpio_disable_interrupt(SX127x_DIO1_PIN);
      hw_gpio_disable_interrupt(SX127x_DIO0_PIN);
  }
#ifdef PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN
    hw_gpio_configure_pin(SX127x_MANUAL_RXTXSW_PIN, false, GPIO_MODE_OUTPUT_PP, 0);
#endif
  #ifdef PLATFORM_SX127X_USE_DIO3_PIN
  hw_gpio_configure_pin(SX127x_DIO3_PIN, true, GPIO_MODE_INPUT, 0);
  if(disable_interrupts)
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
#ifdef PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN
  hw_gpio_configure_pin_stm(SX127x_MANUAL_RXTXSW_PIN, &initStruct);
#endif
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

void __platform_init()
{
    stm32_common_mcu_init();
    __gpio_init();
    __hw_debug_init();

#if defined(USE_SX127X) || defined(USE_NETDEV_DRIVER)
    hw_radio_io_init(true);

  #ifdef PLATFORM_SX127X_USE_RESET_PIN
    hw_radio_reset();
  #endif
#endif

    HAL_EnableDBGSleepMode(); // TODO impact on power?

    // init blockdevices
    // the embedded EEPROM is divided in 2 logical block devices, 1 for metadata and 1 for permanent files
    blockdevice_stm32_eeprom_t* stm32_eeprom_metadata_bd = (blockdevice_stm32_eeprom_t*)metadata_blockdevice;
    stm32_eeprom_metadata_bd->base.driver = &blockdevice_driver_stm32_eeprom;
    stm32_eeprom_metadata_bd->base.offset = 0;
    stm32_eeprom_metadata_bd->base.size = (uint32_t)((uint8_t*)&__d7ap_fs_metadata_end - (uint8_t*)&__d7ap_fs_metadata_start);

    blockdevice_init(metadata_blockdevice);

    blockdevice_stm32_eeprom_t* stm32_eeprom_permanent_files_bd = (blockdevice_stm32_eeprom_t*)persistent_files_blockdevice;
    stm32_eeprom_permanent_files_bd->base.driver = &blockdevice_driver_stm32_eeprom;
    stm32_eeprom_permanent_files_bd->base.offset = (uint32_t)((uint8_t*)&__d7ap_fs_metadata_end - (uint8_t*)&__d7ap_fs_metadata_start); // blockdevices begins after metadata block device // TODO aligment on sector?
    stm32_eeprom_permanent_files_bd->base.size = (uint32_t)((uint8_t*)&__d7ap_fs_permanent_files_end - (uint8_t*)&__d7ap_fs_permanent_files_start);

    blockdevice_init(persistent_files_blockdevice);
    blockdevice_init(volatile_blockdevice);
}

void __platform_post_framework_init()
{
    __ubutton_init();
    led_init();
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
