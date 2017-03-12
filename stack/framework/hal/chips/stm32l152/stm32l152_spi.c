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

/*! \file stm32l152_spi.c
 *
 */

 #include <stdbool.h>
 #include <assert.h>

#include "hwgpio.h"
 #include "hwspi.h"

 #include "platform.h"

#include "stm32l152_pins.h"
#include "stm32l1xx_hal.h"

 #define USARTS    4
#define MAX_SPI_SLAVE_HANDLES 4        // TODO expose this in chip configuration

 typedef struct {
//   pin_id_t mosi;
//   pin_id_t miso;
//   pin_id_t clk;
	 uint8_t id;
   uint32_t pins;
   uint32_t alternate;
   GPIO_TypeDef* port;
   SPI_TypeDef* spi;
 } spi_pins_t;

 struct spi_slave_handle {
   spi_handle_t* spi;
   pin_id_t      cs;
   bool          cs_is_active_low;
   bool          selected;
 };

 // TODO to be completed with all documented locations
 static spi_pins_t usarts[USARTS] = {
   {
     // USART 0

//       .mosi    = { .port = 0xFF, .pin = 0xFF },
//       .miso    = { .port = 0xFF, .pin = 0xFF },
//       .clk     = { .port = 0xFF, .pin = 0xFF },
	   .port 	= 0,
	   .pins 	= 0,
	   .alternate = 0,
	   .spi = 0

   },
   {
        // USART 1

//          .mosi     = { .port = 0, .pin = 7 },
//          .miso     = { .port = 0, .pin = 6 },
//          .clk      = { .port = 0, .pin = 5 },
		  .port 	= GPIOA,
		  .pins 	= GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7,
		  .alternate = GPIO_AF5_SPI1,
		  .spi = SPI1

      },
   {
     // USART 2

//       .mosi     = { .port = 1, .pin = 15 },
//       .miso     = { .port = 1, .pin = 14 },
//       .clk      = { .port = 1, .pin = 13 },
	   .port 	= GPIOB,
	   .pins 	 = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15,
	   .alternate = GPIO_AF5_SPI2,
		  .spi = SPI2

   },
   {
     // USART 3

//       .mosi     = { .port = 2, .pin = 12 },
//       .miso     = { .port = 2, .pin = 11 },
//       .clk      = { .port = 2, .pin = 10 },
	   .port 	= GPIOC,
	   .pins 	 = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12,
	   .alternate = GPIO_AF6_SPI3,
		  .spi = SPI3

   }
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
   uint8_t		id;
 };

 // private storage for handles, pointers to these records are passed around
 static spi_handle_t handle[USARTS] =
 	 	 	 {
 	 	 			 {.id=0, .hspi.Instance=NULL},
					 {.id=1, .hspi.Instance=NULL},
					 {.id=2, .hspi.Instance=NULL},
					 {.id=3, .hspi.Instance=NULL}
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

   switch (spi->id)
      {
      case 1:
   	   __HAL_RCC_SPI1_CLK_ENABLE();
   	   break;
      case 2:
   	   __HAL_RCC_SPI2_CLK_ENABLE();
   	   break;
      case 3:
   	   __HAL_RCC_SPI3_CLK_ENABLE();
   	   break;
      }

  if (HAL_SPI_Init(&(spi->hspi)) != HAL_OK)
	{
	assert ("cannot init");
	return;
	}

   spi->active = true;
 }

 void spi_disable(spi_handle_t* spi) {
   // already inactive?
   if( ! spi->active ) { return; }

   HAL_SPI_DeInit(&spi->hspi);

   switch (spi->id)
	{
	case 1:
	   __HAL_RCC_SPI1_CLK_ENABLE();
	   break;
	case 2:
	   __HAL_RCC_SPI2_CLK_ENABLE();
	   break;
	case 3:
	   __HAL_RCC_SPI3_CLK_ENABLE();
	   break;
	}

   // turn off all CS lines, because bus is down
   // clients should be powered down also
   for(uint8_t s=0; s<spi->slaves; s++) {
     hw_gpio_clr(spi->slave[s]->cs);
   }

   spi->active = false;
 }
  
 spi_handle_t* spi_init(uint8_t uart, uint32_t baudrate, uint8_t databits, bool msbf, uint8_t pins)
 {
   // assert what is supported by HW
   assert(databits == 8);
   assert(pins == 0);
   assert(uart < USARTS);
   assert(uart > 0);

   if (handle[uart].hspi.Instance != NULL)
   {
	   //TODO: check if settings are ok
	   return &handle[uart];
   }

   GPIO_InitTypeDef GPIO_InitStruct;
   GPIO_InitStruct.Pin = usarts[uart].pins;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = usarts[uart].alternate;
   HAL_GPIO_Init(usarts[uart].port, &GPIO_InitStruct);

   handle[uart].hspi.Instance = usarts[uart].spi;
  	handle[uart].hspi.Init.Mode = SPI_MODE_MASTER;
  	handle[uart].hspi.Init.Direction = SPI_DIRECTION_2LINES;
  	handle[uart].hspi.Init.DataSize = SPI_DATASIZE_8BIT;
  	handle[uart].hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
  	handle[uart].hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
  	handle[uart].hspi.Init.NSS = SPI_NSS_SOFT;
  	switch (baudrate)
  	{
  	case 16000000:
  		handle[uart].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  		break;
  	case 8000000:
  		handle[uart].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  		break;
  	case 4000000:
  		handle[uart].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  		break;
  	case 2000000:
  		handle[uart].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  		break;
  	case 1000000:
  		handle[uart].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  		break;
  	case 500000:
  		handle[uart].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  		break;
  	case 250000:
  		handle[uart].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  		break;
  	case 125000:
  		handle[uart].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  		break;
  	default:
  		assert(false);
  	}

  	handle[uart].hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  	if (msbf)
  	handle[uart].hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  	else
  	handle[uart].hspi.Init.FirstBit = SPI_FIRSTBIT_LSB;
  	handle[uart].hspi.Init.TIMode = SPI_TIMODE_DISABLE;
  	handle[uart].hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  	handle[uart].hspi.Init.CRCPolynomial = 10;

  
   return &handle[uart];
 }

 spi_slave_handle_t*  spi_init_slave(spi_handle_t* spi, pin_id_t cs_pin, bool cs_is_active_low) {
	assert(next_spi_slave_handle < MAX_SPI_SLAVE_HANDLES);

	bool initial_level = spi->active > 0 && cs_is_active_low;

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = (1<<cs_pin.pin);
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	error_t err = hw_gpio_configure_pin_stm(cs_pin, &GPIO_InitStruct);
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

 void spi_exchange_bytes(spi_slave_handle_t* slave, uint8_t* TxData, uint8_t* RxData, unsigned int length)
 {
   uint16_t i = 0;
   if( RxData != NULL && TxData != NULL ) {           // two way transmission
	   HAL_SPI_TransmitReceive(&slave->spi->hspi, TxData, RxData, length, HAL_MAX_DELAY);
   } else if( RxData == NULL && TxData != NULL ) {    // send only
	   HAL_SPI_Transmit(&slave->spi->hspi, TxData, length, HAL_MAX_DELAY);
   } else if( RxData != NULL && TxData == NULL ) {   // receive only
	   HAL_SPI_Receive(&slave->spi->hspi, RxData, length, HAL_MAX_DELAY);
   }
 }
