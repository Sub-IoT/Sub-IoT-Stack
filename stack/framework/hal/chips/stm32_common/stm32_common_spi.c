/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2018 University of Antwerp
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

/*! \file stm32_common_spi.c
 *
 */

#include <stdbool.h>
#include <assert.h>

#include "hwspi.h"
#include "stm32_device.h"
#include "stm32_common_mcu.h"
#include "stm32_common_gpio.h"
#include "platform.h"
#include "ports.h"
#include "hwgpio.h"
#include "errors.h"


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
  uint8_t             spi_port_number; // for reference to SPI port defined in ports.h (pins)
};

// private storage for handles, pointers to these records are passed around
static spi_handle_t handle[SPI_COUNT] = {
  {.hspi.Instance=NULL}
};

static DMA_HandleTypeDef dma_rx;
static DMA_HandleTypeDef dma_tx;
static bool rx_busy; // for synchronizing RX until DMA completed
static spi_slave_handle_t* current_slave; // TODO tmp



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

static void init_pins(spi_handle_t* spi) {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = spi_ports[spi->spi_port_number].alternate;

  hw_gpio_configure_pin_stm(spi_ports[spi->spi_port_number].sck_pin, &GPIO_InitStruct);
  hw_gpio_configure_pin_stm(spi_ports[spi->spi_port_number].miso_pin, &GPIO_InitStruct);
  hw_gpio_configure_pin_stm(spi_ports[spi->spi_port_number].mosi_pin, &GPIO_InitStruct);
}

static void deinit_pins(spi_handle_t* spi) {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;

  hw_gpio_configure_pin_stm(spi_ports[spi->spi_port_number].sck_pin, &GPIO_InitStruct);
  hw_gpio_configure_pin_stm(spi_ports[spi->spi_port_number].miso_pin, &GPIO_InitStruct);
  hw_gpio_configure_pin_stm(spi_ports[spi->spi_port_number].mosi_pin, &GPIO_InitStruct);
}

void spi_enable(spi_handle_t* spi) {
  // already active?
  if(spi->active) { return; }

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

  deinit_pins(spi);
  spi->active = false;
}


spi_handle_t* spi_init(uint8_t spi_number, uint32_t baudrate, uint8_t databits, bool msbf, bool half_duplex) {
  // assert what is supported by HW
  assert(databits == 8);
  assert(spi_number < SPI_COUNT);

  next_spi_slave_handle = 0;
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
  handle[spi_number].hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
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

  // configure DMA
  __HAL_RCC_DMA1_CLK_ENABLE(); // TODO disable before sleep

  dma_rx.Instance = spi_ports[spi_number].dma_rx_channel;
  dma_rx.Init.Request = spi_ports[spi_number].dma_rx_request_number;
  dma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  dma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  dma_rx.Init.MemInc = DMA_MINC_ENABLE;
  dma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  dma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  dma_rx.Init.Mode = DMA_NORMAL;
  dma_rx.Init.Priority = DMA_PRIORITY_HIGH;
  HAL_DMA_Init(&dma_rx);
  __HAL_LINKDMA(&handle[spi_number].hspi, hdmarx, dma_rx); // TODO needed?
  HAL_NVIC_SetPriority(spi_ports[spi_number].dma_rx_irq, 1, 0);
  HAL_NVIC_EnableIRQ(spi_ports[spi_number].dma_rx_irq);


  dma_tx.Instance = spi_ports[spi_number].dma_tx_channel;
  dma_tx.Init.Request = spi_ports[spi_number].dma_tx_request_number;
  dma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  dma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  dma_tx.Init.MemInc = DMA_MINC_ENABLE;
  dma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  dma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  dma_tx.Init.Mode = DMA_NORMAL;
  dma_tx.Init.Priority = DMA_PRIORITY_HIGH;
  HAL_DMA_Init(&dma_tx);
  __HAL_LINKDMA(&handle[spi_number].hspi, hdmatx, dma_tx); // TODO needed?
  HAL_NVIC_SetPriority(spi_ports[spi_number].dma_tx_irq, 1, 0);
  HAL_NVIC_EnableIRQ(spi_ports[spi_number].dma_tx_irq);

  // TODO deinit DMA, clock and irq

  spi_enable(&handle[spi_number]);
  return &handle[spi_number];
}

// TODO move
void DMA1_Channel4_5_6_7_IRQHandler(void) {
  HAL_DMA_IRQHandler(current_slave->spi->hspi.hdmarx); // TODO
  HAL_DMA_IRQHandler(current_slave->spi->hspi.hdmatx); // TODO
}

void DMA1_Channel2_3_IRQHandler(void) {
  HAL_DMA_IRQHandler(current_slave->spi->hspi.hdmarx); // TODO
  HAL_DMA_IRQHandler(current_slave->spi->hspi.hdmatx); // TODO
}


void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  //log_print_string("!!! cmplt DMA->ISR %x\n", DMA1->ISR);
  rx_busy = false;
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  //log_print_string("!!! TxRx cmplt DMA->ISR %x\n", DMA1->ISR);
  rx_busy = false;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  //log_print_string("!!! TxRx cmplt DMA->ISR %x\n", DMA1->ISR);
  rx_busy = false;
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  assert(false); // TODO
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

static void wait_until_completed(spi_slave_handle_t* slave) {
  while(__HAL_SPI_GET_FLAG(&slave->spi->hspi, SPI_FLAG_RXNE));
  while(! __HAL_SPI_GET_FLAG(&slave->spi->hspi, SPI_FLAG_TXE));
  while(__HAL_SPI_GET_FLAG(&slave->spi->hspi, SPI_FLAG_BSY));
}

unsigned char spi_exchange_byte(spi_slave_handle_t* slave, unsigned char data) {
  log_print_string("> SPI byte %x\n", data);
  //  log_print_string("SPI SR %x\n", &slave->spi->hspi.Instance->SR);
  //assert(slave->spi->hspi.Instance->SR == 0);
  log_print_string("DMA ISR %x\n", DMA1->ISR);
  __HAL_DMA_CLEAR_FLAG(current_slave->spi->hspi.hdmarx, __HAL_DMA_GET_GI_FLAG_INDEX(current_slave->spi->hspi.hdmarx));
  __HAL_DMA_CLEAR_FLAG(current_slave->spi->hspi.hdmarx, __HAL_DMA_GET_TC_FLAG_INDEX(current_slave->spi->hspi.hdmarx));
  __HAL_DMA_CLEAR_FLAG(current_slave->spi->hspi.hdmarx, __HAL_DMA_GET_TE_FLAG_INDEX(current_slave->spi->hspi.hdmarx));
  __HAL_DMA_CLEAR_FLAG(current_slave->spi->hspi.hdmarx, __HAL_DMA_GET_HT_FLAG_INDEX(current_slave->spi->hspi.hdmarx));
  __HAL_DMA_CLEAR_FLAG(current_slave->spi->hspi.hdmatx, __HAL_DMA_GET_GI_FLAG_INDEX(current_slave->spi->hspi.hdmatx));
  __HAL_DMA_CLEAR_FLAG(current_slave->spi->hspi.hdmatx, __HAL_DMA_GET_TC_FLAG_INDEX(current_slave->spi->hspi.hdmatx));
  __HAL_DMA_CLEAR_FLAG(current_slave->spi->hspi.hdmatx, __HAL_DMA_GET_TE_FLAG_INDEX(current_slave->spi->hspi.hdmatx));
  __HAL_DMA_CLEAR_FLAG(current_slave->spi->hspi.hdmatx, __HAL_DMA_GET_HT_FLAG_INDEX(current_slave->spi->hspi.hdmatx));


  assert(DMA1->ISR == 0);

  uint8_t returnData;
  //while(rx_busy);
  current_slave = slave; // TODO tmp
  rx_busy = true;
  HAL_SPI_TransmitReceive_DMA(&slave->spi->hspi, &data, &returnData, 1);
  //while(rx_busy); // TODO timeout
  wait_until_completed(slave);
  log_print_string("< SPI byte %x\n", returnData);
  return returnData;
}

void spi_send_byte_with_control(spi_slave_handle_t* slave, uint16_t data) {
  while(rx_busy);
  rx_busy = true;
  current_slave = slave; // TODO tmp
  HAL_SPI_Transmit_DMA(&slave->spi->hspi, (uint8_t *)&data, 2);
  while(rx_busy); // TODO timeout
  wait_until_completed(slave);
}

void spi_exchange_bytes(spi_slave_handle_t* slave, uint8_t* TxData, uint8_t* RxData, size_t length) {
  // HACK: in half duplex mode we encountered cases where the SPI clock stays enabled too long, causing extra bytes being received,
  // which are returned on a new receive call. Drop received bytes first

//  log_print_string("> SPI bytes\n");
//  if(TxData)
//    log_print_data(TxData, length);

//  while(slave->spi->hspi.Instance->SR & SPI_SR_RXNE)
//    slave->spi->hspi.Instance->DR;

  assert(slave->spi->hspi.Instance->SR == 0);
  assert(DMA1->ISR == 0);

  while(rx_busy);
  current_slave = slave; // TODO tmp
  rx_busy = true;
  if( RxData != NULL && TxData != NULL ) {           // two way transmission
    HAL_SPI_TransmitReceive_DMA(&slave->spi->hspi, TxData, RxData, length);
  } else if( RxData == NULL && TxData != NULL ) {    // send only
    HAL_SPI_Transmit_DMA(&slave->spi->hspi, TxData, length);
  } else if( RxData != NULL && TxData == NULL ) {   // receive only
    // RX done using DMA, but we block until completed
    //__HAL_SPI_DISABLE(&slave->spi->hspi); // TODO HACK
  log_print_string("!! RX DMA\n");
    HAL_SPI_Receive_DMA(&slave->spi->hspi, RxData, length);
  }

  while(rx_busy); // TODO timeout
  wait_until_completed(slave);
//  log_print_string("< SPI bytes\n");
//  if(RxData)
//    log_print_data(TxData, length);
}

