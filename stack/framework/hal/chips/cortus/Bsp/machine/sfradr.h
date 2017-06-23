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

#define SFRADR_GPIO_EDGE1       0x40004000
#define SFRADR_GPIO_EDGE2       0x40004100 

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

#define IRQ_GPIO_EDGE1         11

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
