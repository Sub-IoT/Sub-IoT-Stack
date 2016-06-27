/*******************************************************************************
* File: sfradr.h
* @section License
* <b>(C) Copyright 2005 Cortus S.A, http://www.cortus.com
*******************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Cortus S.A has no
* obligation to support this Software. Cortus S.A is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Cortus S.A will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
******************************************************************************/

#ifndef _SFRADR_H
#define _SFRADR_H

#define CLOCK_FREQUENCY 	10000000
#define USB2_CLOCK_FREQUENCY    50000000

#define SFRADR_IC               0x40000000
#define SFRADR_IC1              0x40000100

#define SFRADR_CAPINT0          0x40000200
#define SFRADR_CAPINT1          0x40000300
#define SFRADR_CAPINT2          0x40000400
#define SFRADR_CAPINT3          0x40000500

#define SFRADR_UART1            0x40001000   

#define SFRADR_COUNTER1         0x40002000
#define SFRADR_COUNTER2         0x40002200

#define SFRADR_TIMER1           0x40002400
#define SFRADR_TIMER1_CAPA      0X40002500
#define SFRADR_TIMER1_CMPA      0X40002600
#define SFRADR_TIMER1_CMPB      0X40002700

#define SFRADR_WDT              0x40003000
#define SFRADR_WDT_WD           0x40003100

#define SFRADR_GPIO1            0x40004000 
#define SFRADR_GPIO2            0x40004100 

#define SFRADR_SPI              0x40005000 
#define SFRADR_SPI2             0x40005100 

#define SFRADR_I2C              0x40006000 

#define SFRADR_USB              0x40007000 
#define SFRADR_USB_EP0_IN       0x40007100 
#define SFRADR_USB_EP0_OUT      0x40007200 
#define SFRADR_USB_EP1_OUT      0x40007300 
#define SFRADR_USB_EP2_IN       0x40007400 

#define SFRADR_IFS              0x40008000
#define SFRADR_SERIAL           0x40009000

#define SFRADR_DCACHE           0x4000a000

#define SFRADR_BRKPTS           0x50000000   /* IRQ 4 */
#define SFRADR_BRKPTS1          0x50000100

#define IRQ_DEBUGGER_STOP       4

#define IRQ_UART1_TX            5
#define IRQ_UART1_RX            6

#define IRQ_COUNTER1            7

#define IRQ_COUNTER2            8

#define IRQ_ETH_RX              9
#define IRQ_ETH_TX             10

#define IRQ_GPIO1              11

#define IRQ_SPI_TX             12
#define IRQ_SPI_RX             13

#define IRQ_SPI2_TX            14
#define IRQ_SPI2_RX            15

#define IRQ_TIMER1             16
#define IRQ_TIMER1_CAPA        17
#define IRQ_TIMER1_CMPA        18
#define IRQ_TIMER1_CMPB        19

#define IRQ_I2C_TX             20
#define IRQ_I2C_RX             21

#endif
