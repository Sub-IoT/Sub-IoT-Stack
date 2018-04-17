/*********************************************************************************
 * This confidential and proprietary software may be used only as authorized 
 *                      by a licensing agreement from                           
 *                           Cortus S.A.
 *
 *             (C) Copyright 2004, 2005, 2006 Cortus S.A.
 *                           ALL RIGHTS RESERVED
 *
 * The entire notice above must be reproduced on all authorized copies
 * and any such reproduction must be pursuant to a licensing agreement 
 * from Cortus S.A. (http://www.cortus.com)
 *
 * $CortusRelease$
 * $FileName$
 *
 *********************************************************************************/

#ifndef _UART_H
#define _UART_H
#include <machine/sfradr.h>

typedef struct Uart
{
    /* Transmit data fifo */
    volatile unsigned tx_data;
    
    /* Tranmsmit status  
         5:    cts              1 means tx is clear to send 
         4:    empty            1 if fifo is entirely empty 
         3:    3/4 empty        1 if at least 3/4 of fifo is empty
         2:    1/2 empty        1 if at least 1/2 of fifo is empty
         1:    1/4 empty        1 if at least 1/4 of fifo is empty
         0:    !full            1 if fifo is not full */
    volatile unsigned tx_status;

    /* Transmit interrupt mask */
    volatile unsigned tx_mask;

    unsigned __fill0x0c;

    /* Receive data fifo */
    volatile unsigned rx_data;

    /* Receive status
         7:    framing error    1 if missing stop bit
         6:    overrun          1 if fifo overrun has occured
          :       - 
         3:    3/4 full         1 if fifo at least 3/4 full
         2:    1/2 full         1 if fifo at least 1/2 full
         1:    1/4 full         1 if fifo at least 1/4 full
         0:    !empty           1 if some data is available */
    volatile unsigned rx_status;

    /* Receive interrupt mask */
    volatile unsigned rx_mask;
    
    /* Timeout value */
    volatile unsigned rx_timeout;

    /* 
     * The following registers are only present in the full version of the 
     * uart implementation. 
     */

    /* UART configuration register (default 0)
         5:     cts_en          cts enable for TX
         4:     nstop   	0 => 1 stop bit, 1 => 2 stop bits 
     [3:2]:     parity          parity mode
         1:     parity_en       enable parity
         0:     nbits           0 => 8 bit data, 1 => 7 bits data */
     volatile unsigned config;
 
    /* parity mode = parity check mode
             0:   even
             1:   odd
             2:   space
             3:   mark */
    
    /* Clock divider, this is a 16 bits register with
       13 bits for the integer part, 3 bits for the fractional part.
       e.g. 32768Hz clock needs to be divided by 3 3/8 for 9600 (9709) bauds
       The register is 3 3/8 per default. i.e. 0x001b == 3*8+3 */
    volatile unsigned divider;

    /* Clock source selection */
    volatile unsigned selclk;

    /* Uart enable */
    volatile unsigned enable;

} Uart;

#define uart1 ((Uart *)SFRADR_UART1)
#define uart2 ((Uart *)SFRADR_UART2)

/***************  Bit definition for UART_RX_STATUS register ******************/
#define UART_RX_STATUS_FIFO_EMPTY       1 /*!< no data available in the fifo */

/***************  Bit definition for UART_TX_STATUS register ******************/
#define UART_TX_STATUS_FIFO_NOT_EMPTY   1 /*!< space in the fifo */

#endif
