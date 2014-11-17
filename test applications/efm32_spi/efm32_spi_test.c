
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

#include <stdint.h>
#include <string.h>
#include "leds.h"
#include "system.h"
#include "log.h"
#include "timer.h"
#include "efm32gg_spi.h"


/* SPI Data Buffers */
#define SPI_BUFFER_SIZE     128

volatile char spiRxBuffer[SPI_BUFFER_SIZE];
volatile char spiTxBuffer[SPI_BUFFER_SIZE] = "Olleh Dlrow!";

void Timer_Loop(void);

timer_event timer = { .next_event = 512 , .f = Timer_Loop };

volatile int count = 0;

/**************************************************************************//**
 * @brief Timer_Loop
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void Timer_Loop(void)
{
    timer_add_event( &timer );
    led_toggle(0);
    led_toggle(1);

    /* Fill TX buffer */
    sprintf(spiTxBuffer, "[%d] Olleh Dlrow!", count);

    /* Send data */
    spiDmaTransfer((uint8_t*) spiTxBuffer, (uint8_t*) spiRxBuffer, strlen(spiTxBuffer));

    log_print_string("[%d] Hello World!\n\r", count);

    //snprintf(spiTxBuffer, 19,  "%s", spiRxBuffer);

    count++;
}


/**************************************************************************//**
 * @brief  Main function
 * This example sets up the DMA to transfer outbound and incoming data from the
 * SPI (SPI_CHANNEL) to/from the source/destination buffers. Three tests are done:
 * 1) Transmit data (string) without reading received data
 * 2) Transmit data (string) and transfer received data to RAM buffer
 * 3) Transmit dummy data and transfer received data to RAM buffer
 *****************************************************************************/
int main(void)
{ 
  /* Initialize chip */
  //CHIP_Init();
    system_init();
    led_init();
    timer_init();
    led_on(0);
  
    /* Configuring clocks in the Clock Management Unit (CMU) */
    setupCmu();

    /* Configura USART for SPI */
    setupSpi();

    /* Configure DMA transfer from RAM to SPI using ping-pong */      
    setupDma();

    sprintf(spiTxBuffer, "[%04d] Olleh Dlrow!", count);

    /* Start loop */
    Timer_Loop();

    /* Done */
    while (1);
}
