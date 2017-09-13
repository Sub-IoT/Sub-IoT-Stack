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

/*! \file stm32l0xx_spi.c
 *
 */

#include <stdbool.h>
#include <assert.h>

#include "hwgpio.h"
#include "hwspi.h"
#include "mcu.h"
#include "platform.h"
#include "ports.h"

#include "stm32l0xx_hal.h"
#include "stm32l0xx_gpio.h"


#define MAX_SPI_SLAVE_HANDLES 4        // TODO expose this in chip configuration

struct spi_slave_handle {
  spi_handle_t* spi;
  pin_id_t      cs;
  bool          cs_is_active_low;
  bool          selected;
};

uint8_t            next_spi_slave_handle = 0;
spi_slave_handle_t slave_handle[MAX_SPI_SLAVE_HANDLES];

// private implementation of handle struct
struct spi_handle {
  SPI_HandleTypeDef hspi;
  spi_slave_handle_t* slave[MAX_SPI_SLAVE_HANDLES];
  uint8_t             slaves;  // number of slaves for array mgmt
  uint8_t             users;   // for reference counting of active slaves
  bool                active;
};

// private storage for handles, pointers to these records are passed around
static spi_handle_t handle[SPI_COUNT] = {
  {.hspi.Instance=NULL}
};

static void ensure_slaves_deselected(spi_handle_t* spi) {
  // make sure CS lines for all slaves of this bus are high for active low
  // slaves and vice versa
  for(uint8_t s=0; s<spi->slaves; s++) {
    if(spi->slave[s]->cs_is_active_low) {
      hw_gpio_set(spi->slave[s]->cs);
    } else {
      hw_gpio_clr(spi->slave[s]->cs);
    }
  }
}

void spi_enable(spi_handle_t* spi) {
  // already active?
  if(spi->active) { return; }

  // bringing SPI bus up
  ensure_slaves_deselected(spi);

  switch ((uint32_t)(spi->hspi.Instance))
  {
    case SPI1_BASE:
      __HAL_RCC_SPI1_CLK_ENABLE();
      __HAL_RCC_GPIOA_CLK_ENABLE(); // TODO
      __HAL_RCC_GPIOB_CLK_ENABLE(); // TODO
      break;
    case SPI2_BASE:
      __HAL_RCC_SPI2_CLK_ENABLE();
      break;
    default:
      assert(false);
  }

  if (HAL_SPI_Init(&(spi->hspi)) != HAL_OK)
  {
    assert(false);
    return;
  }

  spi->active = true;
}

void spi_disable(spi_handle_t* spi) {
  // already inactive?
  if( ! spi->active ) { return; }

  HAL_SPI_DeInit(&spi->hspi);

  switch ((uint32_t)(spi->hspi.Instance))
  {
    case SPI1_BASE:
      __HAL_RCC_SPI1_CLK_ENABLE();
      break;
    case SPI2_BASE:
      __HAL_RCC_SPI2_CLK_ENABLE();
      break;
    default:
      assert(false);
  }

  // turn off all CS lines, because bus is down
  // clients should be powered down also
  for(uint8_t s=0; s<spi->slaves; s++) {
    hw_gpio_clr(spi->slave[s]->cs);
  }

  spi->active = false;
}


spi_handle_t* spi_init(uint8_t spi_number, uint32_t baudrate, uint8_t databits, bool msbf) {
  // assert what is supported by HW
  assert(databits == 8);
  assert(spi_number < SPI_COUNT);

  if (handle[spi_number].hspi.Instance != NULL)
  {
    //TODO: check if settings are ok
    return &handle[spi_number];
  }

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = spi_ports[spi_number].alternate;

  GPIO_InitStruct.Pin = 1 << GPIO_PIN(spi_ports[spi_number].sck_pin);
  hw_gpio_configure_pin_stm(spi_ports[spi_number].sck_pin, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = 1 << GPIO_PIN(spi_ports[spi_number].miso_pin);
  hw_gpio_configure_pin_stm(spi_ports[spi_number].miso_pin, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = 1 << GPIO_PIN(spi_ports[spi_number].mosi_pin);
  hw_gpio_configure_pin_stm(spi_ports[spi_number].mosi_pin, &GPIO_InitStruct);

  handle[spi_number].hspi.Instance = spi_ports[spi_number].spi;
  handle[spi_number].hspi.Init.Mode = SPI_MODE_MASTER;
  handle[spi_number].hspi.Init.Direction = SPI_DIRECTION_2LINES;
  handle[spi_number].hspi.Init.DataSize = SPI_DATASIZE_8BIT;
  handle[spi_number].hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
  handle[spi_number].hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
  handle[spi_number].hspi.Init.NSS = SPI_NSS_SOFT;

  switch (baudrate)
  {
    case 16000000:
      handle[spi_number].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
      break;
    case 8000000:
      handle[spi_number].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
      break;
    case 4000000:
      handle[spi_number].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
      break;
    case 2000000:
      handle[spi_number].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
      break;
    case 1000000:
      handle[spi_number].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
      break;
    case 500000:
      handle[spi_number].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
      break;
    case 250000:
      handle[spi_number].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
      break;
    case 125000:
      handle[spi_number].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
      break;
    default:
      assert(false);
  }

  if (msbf)
    handle[spi_number].hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  else
    handle[spi_number].hspi.Init.FirstBit = SPI_FIRSTBIT_LSB;

  handle[spi_number].hspi.Init.TIMode = SPI_TIMODE_DISABLE;
  handle[spi_number].hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  handle[spi_number].hspi.Init.CRCPolynomial = 10;

  spi_enable(&handle[spi_number]);
  return &handle[spi_number];
}

spi_slave_handle_t*  spi_init_slave(spi_handle_t* spi, pin_id_t cs_pin, bool cs_is_active_low) {
  assert(next_spi_slave_handle < MAX_SPI_SLAVE_HANDLES);

  bool initial_level = spi->active > 0 && cs_is_active_low;

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = 1 << GPIO_PIN(cs_pin);
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP; // TODO depending on cs_is_active_low?
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  error_t err = hw_gpio_configure_pin_stm(cs_pin, &GPIO_InitStruct);
  __HAL_RCC_GPIOB_CLK_ENABLE(); // TODO
  assert(err == SUCCESS || err == EALREADY);

  if(cs_is_active_low) {
    hw_gpio_set(cs_pin);
  } else {
    hw_gpio_clr(cs_pin);
  }

  slave_handle[next_spi_slave_handle] = (spi_slave_handle_t){
      .spi              = spi,
      .cs               = cs_pin,
      .cs_is_active_low = cs_is_active_low,
      .selected         = false
};

  // add slave to spi for back-reference
  spi->slave[spi->slaves] = &slave_handle[next_spi_slave_handle];
  spi->slaves++;



  next_spi_slave_handle++;
  return &slave_handle[next_spi_slave_handle-1];
}

void spi_select(spi_slave_handle_t* slave) {
  if( slave->selected ) { return; } // already selected

  if(slave->cs_is_active_low) {     // select slave
    hw_gpio_clr(slave->cs);
  } else {
    hw_gpio_set(slave->cs);
  }

  slave->selected = true;           // mark it
}

void spi_deselect(spi_slave_handle_t* slave) {
  if( ! slave->selected ) { return; } // already deselected

  if(slave->cs_is_active_low) {       // deselect slave
    hw_gpio_set(slave->cs);
  } else {
    hw_gpio_clr(slave->cs);
  }

  slave->selected = false;            // unmark it
}
unsigned char spi_exchange_byte(spi_slave_handle_t* slave, unsigned char data) {
  uint8_t returnData;
  HAL_SPI_TransmitReceive(&slave->spi->hspi, &data, &returnData, 1, HAL_MAX_DELAY);
  return returnData;
}

void spi_send_byte_with_control(spi_slave_handle_t* slave, uint16_t data) {
  HAL_SPI_Transmit(&slave->spi->hspi, (uint8_t *)&data, 2, HAL_MAX_DELAY);
}

void spi_exchange_bytes(spi_slave_handle_t* slave, uint8_t* TxData, uint8_t* RxData, unsigned int length) {
  if( RxData != NULL && TxData != NULL ) {           // two way transmission
    HAL_SPI_TransmitReceive(&slave->spi->hspi, TxData, RxData, length, HAL_MAX_DELAY);
  } else if( RxData == NULL && TxData != NULL ) {    // send only
    HAL_SPI_Transmit(&slave->spi->hspi, TxData, length, HAL_MAX_DELAY);
  } else if( RxData != NULL && TxData == NULL ) {   // receive only
    HAL_SPI_Receive(&slave->spi->hspi, RxData, length, HAL_MAX_DELAY);
  }
}
