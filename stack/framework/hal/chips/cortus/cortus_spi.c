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

/*! \file cortus_spi.c
 *
 *  \author soomee
 *
 */


#include <stdbool.h>
#include <stdint.h>

#include "debug.h"
#include "hwspi.h"
#include "hwgpio.h"

#include "machine/spi.h"
#include "machine/gpio.h"

#if defined(FRAMEWORK_LOG_ENABLED)
#include <stdio.h>
#endif



struct spi_handle {
  SPI* spi_sfradr;
};

spi_handle_t handle;

struct spi_slave_handle {
  spi_handle_t* spi;
};

spi_slave_handle_t slave_handle;

static pin_id_t      CURRENT_CS;
static bool          CURRENT_CS_IS_ACTIVE_LOW;

#define TIMING_ADJ



spi_handle_t* spi_init(uint8_t idx, uint32_t baudrate, uint8_t databits,
                       bool msbf, uint8_t pins)
{

   // assert what is supported by cortus
   assert(databits == 8);

   // configure new handle
   spi2->divider = baudrate;
   spi2->master = 1;
   spi2->config = 0x0 | ((0x1&(~msbf)) << 2);
   spi2->selclk = 0; // 0:50MHz, 1:25MHz, 2:12.5MHZ, 3:3.125MHz
   //spi2->clk_en = 1;

   //irq[IRQ_SPI2_RX].ipl = 0;
   //irq[IRQ_SPI2_RX].ien = 0;
   //spi2->rx_mask = 0x0;

   //irq[IRQ_SPI2_TX].ipl = 0;
   //irq[IRQ_SPI2_TX].ien = 0;
   //spi2->tx_mask = 0x0;

   //ic->ien = 1; // start function
 
   //handle = (spi_handle_t){
   //   .spi_sfradr = spi2
   //};

   return &handle;
}

static void ensure_slaves_deselected(spi_handle_t* spi) {
  // assume there is one CC1101 module for one cortus board
 if(CURRENT_CS_IS_ACTIVE_LOW)
   hw_gpio_set(CURRENT_CS);
 else
   hw_gpio_clr(CURRENT_CS);
}

void spi_enable(spi_handle_t* spi) {
   ensure_slaves_deselected(spi);

   //while ((spi2->bus_active & 0x1)) {}
   spi2->clk_en = 1;
}

void spi_disable(spi_handle_t* spi) {
   while ((spi2->bus_active & 0x1)) {}
   spi2->clk_en = 0;

   ensure_slaves_deselected(spi);
}

spi_slave_handle_t* spi_init_slave(spi_handle_t* spi, pin_id_t cs_pin, bool cs_is_active_low) {
  // assume there is one CC1101 module for one cortus board
  bool initial_level = 1;
  hw_gpio_configure_pin(cs_pin, false, gpioModePushPull, initial_level);

  CURRENT_CS = cs_pin;
  CURRENT_CS_IS_ACTIVE_LOW = cs_is_active_low;
}

void spi_select(spi_slave_handle_t* slave) {
  // assume there is one CC1101 module for one cortus board
  spi_enable(slave->spi);

  if(CURRENT_CS_IS_ACTIVE_LOW)
    hw_gpio_clr(CURRENT_CS);
  else
    hw_gpio_set(CURRENT_CS);
}

void spi_deselect(spi_slave_handle_t* slave) {
  // assume there is one CC1101 module for one cortus board
#ifdef TIMING_ADJ
  volatile uint32_t i = 0;
  while (i!=7)   i++;
#endif

  if(CURRENT_CS_IS_ACTIVE_LOW)
    hw_gpio_set(CURRENT_CS);
  else
    hw_gpio_clr(CURRENT_CS);

  spi_disable(slave->spi);
#ifdef TIMING_ADJ
  i = 0;
  while (i!=100)   i++;
#endif
}

unsigned char spi_exchange_byte(spi_slave_handle_t* slave, unsigned char data) {
#ifdef TIMING_ADJ
  volatile uint32_t i = 0;
  while (i!=3)   i++;
#endif

   while ((spi2->tx_status & 0x1) != 1) {} // wait until tx fifo is available
   spi2->tx_data = data;

   while ((spi2->rx_status & 0x1) != 1) {} // wait until rx fifo is available 
   return (uint8_t)spi2->rx_data;
}

void spi_send_byte_with_control(spi_slave_handle_t* slave, uint16_t data) {
   // 9-bit transmission is not supported.
#if defined(FRAMEWORK_LOG_ENABLED)
   printf("CORTUS: spi_send_byte_with_control is called.\n");
#endif
}

void spi_exchange_bytes(spi_slave_handle_t* slave,
                        uint8_t* TxData, uint8_t* RxData, size_t length)
{
  uint16_t i = 0;
  if( RxData != NULL && TxData != NULL ) {           // two way transmition
    while( i < length ) {
      RxData[i] = spi_exchange_byte(slave, TxData[i]);
      i++;
    }
  } else if( RxData == NULL && TxData != NULL ) {    // send only
    while( i < length ) {
      spi_exchange_byte(slave, TxData[i]);
      i++;
    }
  } else if( RxData != NULL && TxData == NULL ) {   // receive only
    while( i < length ) {
      RxData[i] = spi_exchange_byte(slave, 0);
      i++;
    }
  }
}
