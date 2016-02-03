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

/*! \file stm32f4_spi.c
 *
 */

// #include <stdbool.h>
// #include <assert.h>

// #include "em_device.h"
// #include "em_chip.h"
// #include "em_usart.h"
// #include "em_gpio.h"
// #include "em_dma.h"
// #include "em_cmu.h"
// #include "em_emu.h"
// #include "em_int.h"

// #include "hwgpio.h"
// #include "hwspi.h"

// #include "platform.h"

// #define USARTS    3
// #define LOCATIONS 2

// typedef struct {
//   uint32_t location;
//   pin_id_t mosi;
//   pin_id_t miso;
//   pin_id_t clk;
// } spi_pins_t;

// // TODO to be completed with all documented locations
// static spi_pins_t location[USARTS][LOCATIONS] = {
//   {
//     // USART 0
//     {
//       .location = USART_ROUTE_LOCATION_LOC0,
//       .mosi     = { .port = gpioPortE, .pin = 10 },
//       .miso     = { .port = gpioPortE, .pin = 11 },
//       .clk      = { .port = gpioPortE, .pin = 12 }
//     },
//     {
//       .location = USART_ROUTE_LOCATION_LOC1,
//       .mosi     = { .port = gpioPortE, .pin = 7 },
//       .miso     = { .port = gpioPortE, .pin = 6 },
//       .clk      = { .port = gpioPortE, .pin = 5 }
//     }
//   },
//   {
//     // USART 1
//     {
//       .location = USART_ROUTE_LOCATION_LOC0,
//       .mosi     = { .port = gpioPortC, .pin = 0 },
//       .miso     = { .port = gpioPortC, .pin = 1 },
//       .clk      = { .port = gpioPortB, .pin = 7 }
//     },
//     {
//       .location = USART_ROUTE_LOCATION_LOC1,
//       .mosi     = { .port = gpioPortD, .pin = 0 },
//       .miso     = { .port = gpioPortD, .pin = 1 },
//       .clk      = { .port = gpioPortD, .pin = 2 }
//     }
//   },
//   {
//     // USART 2
//     {
//       .location = USART_ROUTE_LOCATION_LOC0,
//       .mosi     = { .port = gpioPortC, .pin = 2 },
//       .miso     = { .port = gpioPortC, .pin = 3 },
//       .clk      = { .port = gpioPortC, .pin = 4 }
//     },
//     {
//       .location = USART_ROUTE_LOCATION_LOC1,
//       .mosi     = { .port = gpioPortB, .pin = 3 },
//       .miso     = { .port = gpioPortB, .pin = 4 },
//       .clk      = { .port = gpioPortB, .pin = 5 }
//     }
//   }
// };

// // private implementation of handle struct
// struct spi_handle {
//   USART_TypeDef*    channel;
//   CMU_Clock_TypeDef clock;
//   spi_pins_t*       pins;
// };

// // private storage for handles, pointers to these records are passed around
// static spi_handle_t handle[USARTS] = {
//   {
//     .channel = USART0,
//     .clock   = cmuClock_USART0
//   },
//   {
//     .channel = USART1,
//     .clock   = cmuClock_USART1
//   },
//   {
//     .channel = USART2,
//     .clock   = cmuClock_USART2
//   },
// };
  
// spi_handle_t* spi_init(uint8_t idx, uint32_t baudrate, uint8_t databits,
//                        uint8_t pins)
// {
//   // assert what is supported by HW and emlib
//   assert(databits == 8 || databits == 9);
//   assert(idx < USARTS);
//   assert(pins < LOCATIONS); // TODO: not more implemented yet

//   // complete handle with pin info
//   handle[idx].pins = &location[idx][pins];
  
//   CMU_ClockEnable(cmuClock_GPIO,    true);
//   CMU_ClockEnable(handle[idx].clock, true);

// 	USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;

//   if(databits == 9) {
//     usartInit.databits = usartDatabits9;
//   } else {
//     usartInit.databits = usartDatabits8;
//   }
//   usartInit.baudrate  = baudrate;
//   usartInit.master    = true;
//   usartInit.msbf      = true;
//   usartInit.clockMode = usartClockMode0;
  
//   USART_InitSync(handle[idx].channel, &usartInit);
//   USART_Enable  (handle[idx].channel, usartEnable);

//   error_t err;
//   err = hw_gpio_configure_pin(handle[idx].pins->mosi, false, gpioModePushPull, 0);
//   assert(err == SUCCESS || err == EALREADY);
  
//   err = hw_gpio_configure_pin(handle[idx].pins->miso, false, gpioModeInput,    0);
//   assert(err == SUCCESS || err == EALREADY);
  
//   err = hw_gpio_configure_pin(handle[idx].pins->clk,  false, gpioModePushPull, 0);
//   assert(err == SUCCESS || err == EALREADY);

//   handle[idx].channel->ROUTE = USART_ROUTE_TXPEN
//                              | USART_ROUTE_RXPEN
//                              | USART_ROUTE_CLKPEN
//                              | handle[idx].pins->location;
  
//   return &handle[idx];
// }

// void spi_init_slave(pin_id_t slave) {
//   error_t err = hw_gpio_configure_pin(slave, false, gpioModePushPull, 1);
//   assert(err == SUCCESS || err == EALREADY);
// }

// void spi_select(pin_id_t slave) {
//   hw_gpio_clr(slave);
// }

// void spi_deselect(pin_id_t slave) {
//   hw_gpio_set(slave);
// }

// unsigned char spi_exchange_byte(spi_handle_t* spi, unsigned char data) {
//   return USART_SpiTransfer(spi->channel, data);
// }

// void spi_send_byte_with_control(spi_handle_t* spi, uint16_t data) {
//   USART_TxExt(spi->channel, data);
// }

// void spi_exchange_bytes(spi_handle_t* spi,
//                         uint8_t* TxData, uint8_t* RxData, unsigned int length)
// {
//   uint16_t i = 0;
//   if( RxData != NULL && TxData != NULL ) {           // two way transmition
//     while( i < length ) {
//       RxData[i] = spi_exchange_byte(spi, TxData[i]);
//       i++;
//     }
//   } else if( RxData == NULL && TxData != NULL ) {    // send only
//     while( i < length ) {
//       spi_exchange_byte(spi, TxData[i]);
//       i++;
//     }
//   } else if( RxData != NULL && TxData == NULL ) {   // recieve only
//     while( i < length ) {
//       RxData[i] = spi_exchange_byte(spi, 0);
//       i++;
//     }
//   }
// }
