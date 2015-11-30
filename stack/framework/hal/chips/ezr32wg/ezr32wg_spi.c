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
 *  \author jeremie@wizzilab.com
 *  \author daniel.vandenakker@uantwerpen.be
 *  \author maarten.weyn@uantwerpen.be
 *
 */


#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_dma.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_int.h"
#include "dmactrl.h"
#include "hwgpio.h"
#include <assert.h>

#include "hwspi.h"
#include "platform.h"

// turn on/off the debug prints
#if 0
#include "log.h"
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)  
#endif


//#define SPI_USE_DMA

//moved to platform.h
/* SPI Channel configuration */
//#define SPI_CHANNEL         USART1                      // SPI Channel
//#define SPI_BAUDRATE        9600                    // SPI Frequency
//#define SPI_CLOCK           cmuClock_USART1             // SPI Clock
//#define SPI_ROUTE_LOCATION  USART_ROUTE_LOCATION_LOC1   // SPI GPIO Routing

///* SPI Ports and Pins for the selected route location above.
// * See the datasheet for the availiable routes and corresponding GPIOs */
//#define SPI_PORT_MOSI       gpioPortD
//#define SPI_PORT_MISO       gpioPortD
//#define SPI_PORT_CLK        gpioPortD
//#define SPI_PORT_CS         gpioPortD
//#define SPI_PIN_MOSI        0
//#define SPI_PIN_MISO        1
//#define SPI_PIN_CLK         2
//#define SPI_PIN_CS          3



#ifdef SPI_USE_DMA

//moved to platform.h
///* DMA Channel configuration */
//#define DMA_CHANNEL_TX      0
//#define DMA_CHANNEL_RX      1
//#define DMA_CHANNELS        2
//#define DMA_REQ_RX          DMAREQ_USART1_RXDATAV
//#define DMA_REQ_TX          DMAREQ_USART1_TXBL

/* DMA Callback structure */
DMA_CB_TypeDef spiCallback;

/* Transfer Flags */
volatile bool rxActive;
volatile bool txActive;


/**************************************************************************//**
 * @brief  Call-back called when transfer is complete
 *****************************************************************************/
void transferComplete(unsigned int channel, bool primary, void *user)
{
  (void) primary;
  (void) user;
  
  /* Clear flag to indicate complete transfer */
  if (channel == DMA_CHANNEL_TX)
  {
    txActive = false;  
  }
  else if (channel == DMA_CHANNEL_RX)
  {
    rxActive = false;
  }
}


/**************************************************************************//**
 * @brief Configure DMA in basic mode for both TX and RX to/from USART
 *****************************************************************************/
void setupDma(void)
{
  /* Initialization structs */
  DMA_Init_TypeDef        dmaInit;
  DMA_CfgChannel_TypeDef  rxChnlCfg;
  DMA_CfgDescr_TypeDef    rxDescrCfg;
  DMA_CfgChannel_TypeDef  txChnlCfg;
  DMA_CfgDescr_TypeDef    txDescrCfg;
  
  /* Initializing the DMA */
  dmaInit.hprot        = 0;
  dmaInit.controlBlock = dmaControlBlock;
  DMA_Init(&dmaInit);
  
  /* Setup call-back function */  
  spiCallback.cbFunc  = transferComplete;
  spiCallback.userPtr = NULL;
  
  /*** Setting up RX DMA ***/

  /* Setting up channel */
  rxChnlCfg.highPri   = false;
  rxChnlCfg.enableInt = true;
  rxChnlCfg.select    = DMA_REQ_RX;
  rxChnlCfg.cb        = &spiCallback;
  DMA_CfgChannel(DMA_CHANNEL_RX, &rxChnlCfg);

  /* Setting up channel descriptor */
  rxDescrCfg.dstInc  = dmaDataInc1;
  rxDescrCfg.srcInc  = dmaDataIncNone;
  rxDescrCfg.size    = dmaDataSize1;
  rxDescrCfg.arbRate = dmaArbitrate1;
  rxDescrCfg.hprot   = 0;
  DMA_CfgDescr(DMA_CHANNEL_RX, true, &rxDescrCfg);
  
  /*** Setting up TX DMA ***/

  /* Setting up channel */
  txChnlCfg.highPri   = false;
  txChnlCfg.enableInt = true;
  txChnlCfg.select    = DMA_REQ_TX;
  txChnlCfg.cb        = &spiCallback;
  DMA_CfgChannel(DMA_CHANNEL_TX, &txChnlCfg);

  /* Setting up channel descriptor */
  txDescrCfg.dstInc  = dmaDataIncNone;
  txDescrCfg.srcInc  = dmaDataInc1;
  txDescrCfg.size    = dmaDataSize1;
  txDescrCfg.arbRate = dmaArbitrate1;
  txDescrCfg.hprot   = 0;
  DMA_CfgDescr(DMA_CHANNEL_TX, true, &txDescrCfg);
}


/**************************************************************************//**
 * @brief  SPI DMA Transfer
 * NULL can be input as txBuffer if tx data to transmit dummy data
 * If only sending data, set rxBuffer as NULL to skip DMA activation on RX
 *****************************************************************************/
void spiDmaTransfer(uint8_t *txBuffer, uint8_t *rxBuffer, unsigned int bytes)
{ 
    DPRINT("SPI WR %d bytes", bytes);
  /* Only activate RX DMA if a receive buffer is specified */  
  if (rxBuffer != NULL)
  {
    /* Setting flag to indicate that RX is in progress
     * will be cleared by call-back function */
    rxActive = true;
    
    /* Clear RX regsiters */
    SPI_CHANNEL->CMD = USART_CMD_CLEARRX;
    
    /* Activate RX channel */
    DMA_ActivateBasic(DMA_CHANNEL_RX,
                      true,
                      false,
                      rxBuffer,
                      (void *)&(SPI_CHANNEL->RXDATA),
                      bytes - 1); 
  }
  /* Setting flag to indicate that TX is in progress
   * will be cleared by call-back function */
  txActive = true;
  
  /* Clear TX regsiters */
  SPI_CHANNEL->CMD = USART_CMD_CLEARTX;
  
  /* Activate TX channel */
  DMA_ActivateBasic(DMA_CHANNEL_TX,
                    true,
                    false,
                    (void *)&(SPI_CHANNEL->TXDATA),
                    txBuffer,
                    bytes - 1); 
}


/**************************************************************************//**
 * @brief  Returns if an SPI transfer is active
 *****************************************************************************/
bool spiDmaIsActive(void)
{
  bool temp;
  temp = rxActive;
  temp = temp | txActive;
  return temp;
}


/**************************************************************************//**
 * @brief  Sleep in EM1 until DMA transfer is done
 *****************************************************************************/
void sleepUntilDmaDone(void)
{
  /* Enter EM1 while DMA transfer is active to save power. Note that
   * interrupts are disabled to prevent the ISR from being triggered
   * after checking the transferActive flag, but before entering
   * sleep. If this were to happen, there would be no interrupt to wake
   * the core again and the MCU would be stuck in EM1. While the 
   * core is in sleep, pending interrupts will still wake up the 
   * core and the ISR will be triggered after interrupts are enabled
   * again. 
   */ 

    
    bool isActive = false;

    while(1)
    {
        INT_Disable();
        isActive = spiDmaIsActive();
        if ( isActive )
        {
            EMU_EnterEM1();
        }
        INT_Enable();

        // Exit the loop if transfer has completed
        if ( !isActive )
        {
            DPRINT("SPI DONE");
            break;
        }
    }
}

#endif

/**************************************************************************//**
 * @brief  Enabling clocks
 *****************************************************************************/
void setupCmu(void)
{  
    /* Enabling clocks */
#ifdef SPI_USE_DMA
    CMU_ClockEnable(cmuClock_DMA, true);
#endif
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(SPI_CLOCK, true);
}


/**************************************************************************//**
 * @brief  Setup SPI as Master
 *****************************************************************************/
void setupSpi(void)
{
    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;  

    /* Initialize SPI */
    usartInit.databits  = usartDatabits8;   /* 8 bits of data */
    usartInit.baudrate  = SPI_BAUDRATE;     /* Clock frequency */
    usartInit.master    = true;             /* Master mode */
    usartInit.msbf      = true;             /* Most Significant Bit first */
    usartInit.clockMode = usartClockMode0;  /* Clock idle low, sample on rising edge */

    USART_InitSync(SPI_CHANNEL, &usartInit);

    /* Enable SPI transmit and receive */
    USART_Enable(SPI_CHANNEL, usartEnable);

    /* Configure GPIO pins for SPI */
    //GPIO_PinModeSet(SPI_PORT_MOSI,  SPI_PIN_MOSI,   gpioModePushPull,   0); /* MOSI */
    //GPIO_PinModeSet(SPI_PORT_MISO,  SPI_PIN_MISO,   gpioModeInput,      0); /* MISO */
    //GPIO_PinModeSet(SPI_PORT_CLK,   SPI_PIN_CLK,    gpioModePushPull,   0); /* CLK */
    //GPIO_PinModeSet(SPI_PORT_CS,    SPI_PIN_CS,     gpioModePushPull,   1); /* CS */
    //edit: do this via the hw_gpio_configure_pin interface to signal that the pins are in use
    //note: normally this should be done in the platform-specific initialisation code BUT, since this is a driver for a device (uart) that is
    //an integral part of the MCU we are certain this code will NOT be used in combination with a different MCU so we can do this here
    error_t err;
    err = hw_gpio_configure_pin(SPI_PIN_MOSI, false,   gpioModePushPull,   0); assert(err == SUCCESS); /* MOSI */
    err = hw_gpio_configure_pin(SPI_PIN_MISO, false,   gpioModeInput,      0); assert(err == SUCCESS); /* MISO */
    err = hw_gpio_configure_pin(SPI_PIN_CLK, false,    gpioModePushPull,   0); assert(err == SUCCESS); /* CLK */
    err = hw_gpio_configure_pin(SPI_PIN_CS, false,     gpioModePushPull,   1); assert(err == SUCCESS); /* CS */

    /* Enable routing for SPI pins from USART to location 1 */
    SPI_CHANNEL->ROUTE =  USART_ROUTE_TXPEN |
                          USART_ROUTE_RXPEN |
                          USART_ROUTE_CLKPEN |
                          SPI_ROUTE_LOCATION;
}


// *****************************************************************************
// @fn          spi_init
// @brief       Initialize SPI
// @param       none
// @return      none
// *****************************************************************************
void spi_init(void)
{
    setupCmu();
    setupSpi();
#ifdef SPI_USE_DMA
    setupDma();
#endif
    DPRINT("SPI Init.");
}

// *****************************************************************************
// @fn          spi_auto_cs_on
// @brief       Enable auto Chip Select
// @param       none
// @return      none
// *****************************************************************************
void spi_auto_cs_on(void)
{
    SPI_CHANNEL->CTRL |= USART_CTRL_AUTOCS;
    SPI_CHANNEL->ROUTE |= USART_ROUTE_CSPEN;
}

// *****************************************************************************
// @fn          spi_auto_cs_off
// @brief       Disable auto Chip Select
// @param       none
// @return      none
// *****************************************************************************
void spi_auto_cs_off(void)
{
    SPI_CHANNEL->CTRL &= ~USART_CTRL_AUTOCS;
    SPI_CHANNEL->ROUTE &= ~USART_ROUTE_CSPEN;
}

// *****************************************************************************
// @fn          spi_select_chip
// @brief       Select the chip
// @param       none
// @return      none
// *****************************************************************************
void spi_select_chip(void)
{
	hw_gpio_clr(SPI_PIN_CS);
	//GPIO_PinOutClear( SPI_PORT_CS, SPI_PIN_CS );
}

// *****************************************************************************
// @fn          spi_deselect_chip
// @brief       Deselect the chip
// @param       none
// @return      none
// *****************************************************************************
void spi_deselect_chip(void)
{
	hw_gpio_set(SPI_PIN_CS);
    //GPIO_PinOutSet( SPI_PORT_CS, SPI_PIN_CS );
}

// *****************************************************************************
// @fn          spi_byte
// @brief       Write a byte through SPI
// @param       unsigned char data      Byte to send
// @return      unsigned char           Recieved byte
// *****************************************************************************
unsigned char spi_byte(unsigned char data)
{
#ifdef SPI_USE_DMA
    unsigned char receive = NULL;
    spiDmaTransfer( (uint8_t*) &data, (uint8_t*) &receive, 1 );
    sleepUntilDmaDone();
    return receive;
#else
    return USART_SpiTransfer( SPI_CHANNEL, data );
#endif
}

// *****************************************************************************
// @fn          spi_string
// @brief       Write a string through SPI
// @param       unsigned char* TxData   String to send
// @param       unsigned char* RxData   Reception buffer
// @param       unsigned int length     Length of the string
// @return      none
// *****************************************************************************
void spi_string(unsigned char* TxData, unsigned char* RxData, unsigned int length)
{
#ifdef SPI_USE_DMA
    spiDmaTransfer( (uint8_t*) TxData, (uint8_t*) RxData, length );
    sleepUntilDmaDone();
#else
    uint16_t i = 0;
    if( RxData != NULL && TxData != NULL ) // two way transmition
    {
        while( i < length )
        {
            RxData[i] = spi_byte( TxData[i] );
            i++;
        }
    }
    else if( RxData == NULL && TxData != NULL ) // send only
    {
        while( i < length )
        {
            spi_byte( TxData[i] );
            i++;
        }
    }
    else if( RxData != NULL && TxData == NULL ) // recieve only
    {
        while( i < length )
        {
            RxData[i] = spi_byte( 0 );
            i++;
        }
    }
#endif
}
