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

/*! \file efm32gg_spi.c
 *
 * \author jeremie@wizzilab.com
 * \author daniel.vandenakker@uantwerpen.be
 * \author contact@christophe.vg
 */

#include <stdbool.h>
#include <assert.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_dma.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_int.h"

#include "hwgpio.h"
#include "hwspi.h"

#include "platform.h"

static USART_TypeDef* usart[3] = {
  USART0,
  USART1,
  USART2
};

static CMU_Clock_TypeDef clock[3] = {
  cmuClock_USART0,
  cmuClock_USART1,
  cmuClock_USART2
};

typedef struct {
  uint32_t location;
  pin_id_t mosi;
  pin_id_t miso;
  pin_id_t clk;
} spi_location_t;

// TODO to be completed
static spi_location_t location[2][2] = {
  {
    // USART 0
    {
      .location = USART_ROUTE_LOCATION_LOC0,
      .mosi     = { .port = gpioPortE, .pin = 10 },
      .miso     = { .port = gpioPortE, .pin = 11 },
      .clk      = { .port = gpioPortE, .pin = 12 }
    },
    {
      .location = USART_ROUTE_LOCATION_LOC1,
      .mosi     = { .port = gpioPortE, .pin = 7 },
      .miso     = { .port = gpioPortE, .pin = 6 },
      .clk      = { .port = gpioPortE, .pin = 5 }
    }
  },
  {
    // USART 1
    {
      .location = USART_ROUTE_LOCATION_LOC0,
      .mosi     = { .port = gpioPortC, .pin = 0 },
      .miso     = { .port = gpioPortC, .pin = 1 },
      .clk      = { .port = gpioPortB, .pin = 7 }
    },
    {
      .location = USART_ROUTE_LOCATION_LOC1,
      .mosi     = { .port = gpioPortD, .pin = 0 },
      .miso     = { .port = gpioPortD, .pin = 1 },
      .clk      = { .port = gpioPortD, .pin = 2 }
    }
  }
};
  
void spi_init(spi_definition_t spi) {
  assert(spi.databits == 8 || spi.databits == 9);
  assert(spi.usart < 3);

  assert(spi.location < 2); // TODO: not more implemented yet
  
  CMU_ClockEnable(cmuClock_GPIO,    true);
  CMU_ClockEnable(clock[spi.usart], true);

	USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;

  if(spi.databits == 9) {
    usartInit.databits = usartDatabits9;
  } else {
    usartInit.databits = usartDatabits8;
  }
  usartInit.baudrate  = spi.baudrate;
  usartInit.master    = true;
  usartInit.msbf      = true;
  usartInit.clockMode = usartClockMode0;
  
  USART_InitSync(usart[spi.usart], &usartInit);
  USART_Enable  (usart[spi.usart], usartEnable);

  assert( hw_gpio_configure_pin(location[spi.usart][spi.location].mosi, false, gpioModePushPull, 0) == SUCCESS);
  assert( hw_gpio_configure_pin(location[spi.usart][spi.location].miso, false, gpioModeInput,    0) == SUCCESS);
  assert( hw_gpio_configure_pin(location[spi.usart][spi.location].clk,  false, gpioModePushPull, 0) == SUCCESS);

  usart[spi.usart]->ROUTE = USART_ROUTE_TXPEN
                          | USART_ROUTE_RXPEN
                          | USART_ROUTE_CLKPEN
                          | location[spi.usart][spi.location].location;
}

void spi_init_slave(pin_id_t slave) {
  assert( hw_gpio_configure_pin(slave, false, gpioModePushPull, 1) == SUCCESS);
}

unsigned char spi_byte(uint8_t channel, unsigned char data) {
  return USART_SpiTransfer(usart[channel], data);
}

void spi_send(uint8_t channel, uint16_t data) {
  USART_TxExt(usart[channel], data);
}

void spi_string(uint8_t channel,
                unsigned char* TxData, unsigned char* RxData,
                unsigned int length)
{
  uint16_t i = 0;
  if( RxData != NULL && TxData != NULL ) {           // two way transmition
    while( i < length ) {
      RxData[i] = spi_byte(channel, TxData[i]);
      i++;
    }
  } else if( RxData == NULL && TxData != NULL ) {    // send only
    while( i < length ) {
      spi_byte(channel, TxData[i]);
      i++;
    }
  } else if( RxData != NULL && TxData == NULL ) { // recieve only
    while( i < length ) {
      RxData[i] = spi_byte(channel, 0);
      i++;
    }
  }
}
