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

/*! \file kl02z_spi.c
 *  \author glenn.ergeerts@uantwerpen.be
 */


#include <stdbool.h>
#include "debug.h"

#include "hwspi.h"
#include "platform.h"
#include <MKL02Z4.h>

// turn on/off the debug prints
#if 0
#include "log.h"
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)  
#endif

// private implementation of handle struct
struct spi_handle {
    // TODO empty for now, add needed fields when supporting multiple SPI channels
};


/**************************************************************************//**
 * @brief  Enabling clocks
 *****************************************************************************/
//void setupCmu(void)
//{
//// TODO
////    /* Enabling clocks */
////#ifdef SPI_USE_DMA
////    CMU_ClockEnable(cmuClock_DMA, true);
////#endif
////    CMU_ClockEnable(cmuClock_GPIO, true);
////    CMU_ClockEnable(SPI_CLOCK, true);
//}


/**************************************************************************//**
 * @brief  Setup SPI as Master
 *****************************************************************************/
//void setupSpi(void)
//{
//// TODO
////    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;
////
////    /* Initialize SPI */
////    usartInit.databits  = usartDatabits8;   /* 8 bits of data */
////    usartInit.baudrate  = SPI_BAUDRATE;     /* Clock frequency */
////    usartInit.master    = true;             /* Master mode */
////    usartInit.msbf      = true;             /* Most Significant Bit first */
////    usartInit.clockMode = usartClockMode0;  /* Clock idle low, sample on rising edge */
////
////    USART_InitSync(SPI_CHANNEL, &usartInit);
////
////    /* Enable SPI transmit and receive */
////    USART_Enable(SPI_CHANNEL, usartEnable);
////
////    /* Configure GPIO pins for SPI */
////    //GPIO_PinModeSet(SPI_PORT_MOSI,  SPI_PIN_MOSI,   gpioModePushPull,   0); /* MOSI */
////    //GPIO_PinModeSet(SPI_PORT_MISO,  SPI_PIN_MISO,   gpioModeInput,      0); /* MISO */
////    //GPIO_PinModeSet(SPI_PORT_CLK,   SPI_PIN_CLK,    gpioModePushPull,   0); /* CLK */
////    //GPIO_PinModeSet(SPI_PORT_CS,    SPI_PIN_CS,     gpioModePushPull,   1); /* CS */
////    //edit: do this via the hw_gpio_configure_pin interface to signal that the pins are in use
////    //note: normally this should be done in the platform-specific initialisation code BUT, since this is a driver for a device (uart) that is
////    //an integral part of the MCU we are certain this code will NOT be used in combination with a different MCU so we can do this here
////    error_t err;
////    err = hw_gpio_configure_pin(SPI_PIN_MOSI, false,   gpioModePushPull,   0); assert(err == SUCCESS); /* MOSI */
////    err = hw_gpio_configure_pin(SPI_PIN_MISO, false,   gpioModeInput,      0); assert(err == SUCCESS); /* MISO */
////    err = hw_gpio_configure_pin(SPI_PIN_CLK, false,    gpioModePushPull,   0); assert(err == SUCCESS); /* CLK */
////    err = hw_gpio_configure_pin(SPI_PIN_CS, false,     gpioModePushPull,   1); assert(err == SUCCESS); /* CS */
////
////    /* Enable routing for SPI pins from USART to location 1 */
////    SPI_CHANNEL->ROUTE =  USART_ROUTE_TXPEN |
////                          USART_ROUTE_RXPEN |
////                          USART_ROUTE_CLKPEN |
////                          SPI_ROUTE_LOCATION;
//}


spi_handle_t* spi_init(uint8_t uart, uint32_t baudrate, uint8_t databits, uint8_t pins)
{
    SPI0->C1 = (0<<SPI_C1_SPIE_SHIFT)|(1<<SPI_C1_SPE_SHIFT)|(0<<SPI_C1_SPTIE_SHIFT)|(1<<SPI_C1_MSTR_SHIFT)|(0<<SPI_C1_CPOL_SHIFT)|(0<<SPI_C1_CPHA_SHIFT)|(0<<SPI_C1_SSOE_SHIFT)|(0<<SPI_C1_LSBFE_SHIFT);
    SPI0->C2 = (0<<SPI_C2_SPMIE_SHIFT)|(0<<SPI_C2_MODFEN_SHIFT)|(0<<SPI_C2_BIDIROE_SHIFT)|(1<<SPI_C2_SPISWAI_SHIFT)|(0<<SPI_C2_SPC0_SHIFT);
    SPI0->BR = (0<<SPI_BR_SPPR_SHIFT)|(0<<SPI_BR_SPR_SHIFT);//.4 MHz (fastest available in VLPR-bus clock is limited to .8MHz)
    SPI0->C1 &= ~((1<<SPI_C1_CPOL_SHIFT)|(1<<SPI_C1_CPHA_SHIFT));
    SPI0->C1 |= (0 << SPI_C1_CPHA_SHIFT);
    return NULL;
}

void spi_select(pin_id_t slave)
{
    // TODO support multiple slaves
	hw_gpio_clr(SPI_PIN_CS);
}

void spi_deselect(pin_id_t slave)
{
    // TODO support multiple slaves
	hw_gpio_set(SPI_PIN_CS);
}

uint8_t spi_exchange_byte(spi_handle_t* handle, uint8_t data)
{
    // TODO support multiple slaves
    uint8_t retVal;

    while(!(SPI0->S & SPI_S_SPTEF_MASK));
    SPI0->D = data;
    while(!(SPI0->S & SPI_S_SPRF_MASK));//block until clocking stops so that we cannot accidentally call spi again too soon and cause an error
    retVal = SPI0->D;//read data register to clear SPRF

    return retVal;
}

void spi_exchange_bytes(spi_handle_t* handle, uint8_t* TxData, uint8_t* RxData, size_t length)
{
    // TODO support multiple slaves
    uint16_t i = 0;
    if( RxData != NULL && TxData != NULL ) // two way transmition
    {
        while( i < length )
        {
            RxData[i] = spi_exchange_byte(handle, TxData[i]);
            i++;
        }
    }
    else if( RxData == NULL && TxData != NULL ) // send only
    {
        while( i < length )
        {
            spi_exchange_byte(handle, TxData[i]);
            i++;
        }
    }
    else if( RxData != NULL && TxData == NULL ) // receive only
    {
        while( i < length )
        {
            RxData[i] = spi_exchange_byte(handle, 0);
            i++;
        }
    }
}

