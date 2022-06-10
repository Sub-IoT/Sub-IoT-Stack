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

/*! \file stm32_common_spi.c
 *
 */

#include <stdbool.h>

#include "debug.h"
#include "hwspi.h"
#include "stm32_device.h"
#include "stm32_common_mcu.h"
#include "stm32_common_gpio.h"
#include "platform.h"
#include "ports.h"
#include "hwgpio.h"
#include "errors.h"
#include "hwatomic.h"
#include "hwsystem.h"


#define MAX_SPI_SLAVE_HANDLES 5        // TODO expose this in chip configuration

#define __SPI_DIRECTION_1LINE_RX(__HANDLE__) do {\
                                             CLEAR_BIT((__HANDLE__)->Instance->CR1, SPI_CR1_RXONLY | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE);\
                                             SET_BIT((__HANDLE__)->Instance->CR1, SPI_CR1_BIDIMODE);\
                                             } while(0);


struct spi_slave_handle {
  spi_handle_t* spi;
  pin_id_t      cs;
  bool          cs_is_active_low;
  bool          cs_to_input_if_not_used;
  bool          selected;
};

uint8_t            next_spi_slave_handle = 0;
spi_slave_handle_t slave_handle[MAX_SPI_SLAVE_HANDLES];

// private implementation of handle struct
struct spi_handle {
  SPI_HandleTypeDef hspi;
  spi_slave_handle_t* slave[MAX_SPI_SLAVE_HANDLES];
  uncoupler_handle_t*  uhandle;
  uint8_t             slaves;  // number of slaves for array mgmt
  uint8_t             users;   // for reference counting of active slaves
  bool                active;
  uint8_t             spi_port_number; // for reference to SPI port defined in ports.h (pins)
};

// private storage for handles, pointers to these records are passed around
static spi_handle_t handle[SPI_COUNT] = {
  {.hspi.Instance=NULL}
};

static void configure_cs(spi_slave_handle_t* spi_slave, bool on)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if (on)
  {
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP; // TODO depending on cs_is_active_low?
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  }
  else
  {
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  }
  error_t err = hw_gpio_configure_pin_stm(spi_slave->cs, &GPIO_InitStruct);
  assert(err == SUCCESS || err == EALREADY);
}

static void ensure_slaves_deselected(spi_handle_t* spi) {
  // make sure CS lines for all slaves of this bus are high for active low
  // slaves and vice versa
  for(uint8_t s=0; s<spi->slaves; s++) {
    if(spi->slave[s]->cs_to_input_if_not_used)
    {
      configure_cs(spi->slave[s], false);
    }
    else if(spi->slave[s]->cs_is_active_low) {
      hw_gpio_set(spi->slave[s]->cs);
    } else {
      hw_gpio_clr(spi->slave[s]->cs);
    }
  }
}

static void init_pins(spi_handle_t* spi) {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = spi_ports[spi->spi_port_number].sck_alternate;
  if(spi->hspi.Init.CLKPolarity == SPI_POLARITY_HIGH)
  {
    GPIO_InitStruct.Pull = GPIO_PULLUP;
  }
  hw_gpio_configure_pin_stm(spi_ports[spi->spi_port_number].sck_pin, &GPIO_InitStruct);

  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Alternate = spi_ports[spi->spi_port_number].miso_alternate;
  hw_gpio_configure_pin_stm(spi_ports[spi->spi_port_number].miso_pin, &GPIO_InitStruct);
  GPIO_InitStruct.Alternate = spi_ports[spi->spi_port_number].mosi_alternate;
  hw_gpio_configure_pin_stm(spi_ports[spi->spi_port_number].mosi_pin, &GPIO_InitStruct);
}

void spi_enable(spi_handle_t* spi) {
  // already active?
  if(spi->active) { return; }

  if(spi->uhandle)
  {
    spi->uhandle->driver->uncoupler_set(spi->uhandle, true);
  }

  init_pins(spi);

  // bringing SPI bus up
  ensure_slaves_deselected(spi);

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
      __HAL_RCC_SPI1_CLK_DISABLE();
      break;
    case SPI2_BASE:
      __HAL_RCC_SPI2_CLK_DISABLE();
      break;
    default:
      assert(false);
  }

  // turn off all CS lines
  ensure_slaves_deselected(spi);

  if(spi->uhandle)
  {
    spi->uhandle->driver->uncoupler_set(spi->uhandle, false);
  }

  spi->active = false;
}


spi_handle_t* spi_init(uint8_t spi_number, uint32_t baudrate, uint8_t databits, bool msbf, bool half_duplex, bool cpol, bool cpha, uncoupler_handle_t* uhandle) {
  // assert what is supported by HW
  assert(databits == 8);
  assert(spi_number < SPI_COUNT);

  handle[spi_number].slaves=0;
  
  if (handle[spi_number].hspi.Instance != NULL)
  {
    //TODO: check if settings are ok
    return &handle[spi_number];
  }

  handle[spi_number].spi_port_number = spi_number;
  
  init_pins(&handle[spi_number]);

  handle[spi_number].hspi.Instance = spi_ports[spi_number].spi;
  handle[spi_number].hspi.Init.Mode = SPI_MODE_MASTER;
  if(half_duplex)
    handle[spi_number].hspi.Init.Direction = SPI_DIRECTION_1LINE;
  else
    handle[spi_number].hspi.Init.Direction = SPI_DIRECTION_2LINES;

  handle[spi_number].hspi.Init.DataSize = SPI_DATASIZE_8BIT;
  handle[spi_number].hspi.Init.CLKPhase = SPI_PHASE_1EDGE;

  if(cpol)
    handle[spi_number].hspi.Init.CLKPolarity = SPI_POLARITY_HIGH;
  else
    handle[spi_number].hspi.Init.CLKPolarity = SPI_POLARITY_LOW;

  if(cpha)
    handle[spi_number].hspi.Init.CLKPhase = SPI_PHASE_2EDGE;
  else
    handle[spi_number].hspi.Init.CLKPhase = SPI_PHASE_1EDGE;

  handle[spi_number].hspi.Init.NSS = SPI_NSS_SOFT;

  // TODO take pheripal clock freq into account ...
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

  handle[spi_number].uhandle = uhandle;

  spi_enable(&handle[spi_number]);
  return &handle[spi_number];
}

spi_slave_handle_t*  spi_init_slave(spi_handle_t* spi, pin_id_t cs_pin, bool cs_is_active_low, bool cs_to_input_if_not_used) {
  assert(next_spi_slave_handle < MAX_SPI_SLAVE_HANDLES);

  slave_handle[next_spi_slave_handle] = (spi_slave_handle_t){
      .spi                     = spi,
      .cs                      = cs_pin,
      .cs_is_active_low        = cs_is_active_low,
      .cs_to_input_if_not_used = cs_to_input_if_not_used,
      .selected                = false
  };

  // add slave to spi for back-reference
  spi->slave[spi->slaves] = &slave_handle[next_spi_slave_handle];
  spi->slaves++;

  next_spi_slave_handle++;

  if(!cs_to_input_if_not_used)
  {
    configure_cs(&slave_handle[next_spi_slave_handle-1], true);

    if(cs_is_active_low) {
      hw_gpio_set(cs_pin);
    } else {
      hw_gpio_clr(cs_pin);
    }
  }

  return &slave_handle[next_spi_slave_handle-1];
}

void spi_select(spi_slave_handle_t* slave) {
  if( slave->selected ) { return; } // already selected

  if(slave->cs_to_input_if_not_used)
  {
    configure_cs(slave, true);
  }

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

  if(slave->cs_to_input_if_not_used)
  {
    configure_cs(slave, false);
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

static void spi_read_3wire_bytes(spi_slave_handle_t* slave, uint8_t* address, uint8_t* rxData, size_t length) {
  HAL_SPI_Transmit(&slave->spi->hspi, address, 1, HAL_MAX_DELAY);

  start_atomic();
  // in 3 wire mode RX the SPI pheriperal seems unable to stock clocking after receiving the expected number of bytes
  // disabling the SPI by polling until RXNE is set will result in too many clocks being generated, causing the slave to clokc out more bytes then needed.
  // This can be problematic for some SPI slaves for example when reading a FIFO, where the slave will return bytes from the FIFO but these will be missed by the master.
  // The workaround used by STM in the LMS303 sample of the STM32L4 discovery board is to disable the clock manually after a few cycles, instead of polling RXNE.
  // We use a similar implementation for now
  __SPI_DIRECTION_1LINE_RX(&slave->spi->hspi);
  for (size_t i = 0; i < length; i++) {
    __HAL_SPI_ENABLE(&slave->spi->hspi);
    __DSB();
    __DSB();
    __DSB();
    __DSB();
    __DSB();
    __DSB();
    __DSB();
    __DSB();
    __DSB();
    __DSB();
    __HAL_SPI_DISABLE(&slave->spi->hspi);
    while(!(slave->spi->hspi.Instance->SR & SPI_FLAG_RXNE));
    rxData[i] = *((__IO uint8_t *)&slave->spi->hspi.Instance->DR);
    while((slave->spi->hspi.Instance->SR & SPI_FLAG_BSY) == SPI_FLAG_BSY);
  }
  end_atomic();

  assert(!__HAL_SPI_GET_FLAG(&slave->spi->hspi, SPI_FLAG_OVR));
}

void spi_exchange_bytes(spi_slave_handle_t* slave, uint8_t* TxData, uint8_t* RxData, size_t length) {
  // TODO replace HAL calls with direct registry access for performance / code size ?
  if( RxData != NULL && TxData != NULL ) {
    if(slave->spi->hspi.Init.Direction == SPI_DIRECTION_1LINE) //1line read bytes
      spi_read_3wire_bytes(slave, TxData, RxData, length);
    else // two way transmission
      HAL_SPI_TransmitReceive(&slave->spi->hspi, TxData, RxData, length, HAL_MAX_DELAY);
  } else if( RxData == NULL && TxData != NULL ) {    // send only
    HAL_SPI_Transmit(&slave->spi->hspi, TxData, length, HAL_MAX_DELAY);
  } else if( RxData != NULL && TxData == NULL ) {   // receive only
    if(slave->spi->hspi.Init.Direction == SPI_DIRECTION_2LINES) {
      HAL_SPI_Receive(&slave->spi->hspi, RxData, length, HAL_MAX_DELAY);
    } else {
      assert(false); //1line direction should exchange address as txData and the buffer as rxdata
    }
  }
}
