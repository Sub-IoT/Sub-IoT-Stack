/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>

#include "hwi2c.h"
#include "errors.h"

#include "machine/i2c.h"

#if defined(FRAMEWORK_LOG_ENABLED)
#include <stdio.h>
#endif



typedef struct i2c_handle {
} i2c_handle_t;

static i2c_handle_t handle;

volatile unsigned int addr_buf;
volatile unsigned int tmp;



i2c_handle_t* i2c_init(uint8_t idx, uint8_t pins, uint32_t baudrate, bool pullup) {
    i2c->divider = 542;
    i2c->master = 1;
    i2c->selclk = 0; // 0:50MHz, 1:25MHz, 2:12.5MHZ, 3:3.125MHz
    //i2c->onbus = 1;

    i2c->rx_mask = 0;
    i2c->tx_mask = 0;
    i2c->clk_en = 1;

   return &handle;
}

int8_t i2c_write(i2c_handle_t* handle, uint8_t to, uint8_t* payload, int length) {
    int i;
#if defined(FRAMEWORK_LOG_ENABLED)
    printf ("Start of transmission\n");
#endif
    while (i2c->activity == 1) {}
    i2c->onbus = 1;
    /* Prepare the buffer with correct data, first the address byte */
    addr_buf = (1 << 8) | (to << 1) | 0x0;
    while  ((i2c->tx_status & 0x1) != 1){}  // As long as the fifo is full
    i2c->tx_data = addr_buf;  
    for (i = 0; i < length; i++) 
    {
	while  ((i2c->tx_status & 0x1) != 1){} // As long as the fifo is full
	i2c->tx_data = payload[i];	
    }
    while (!((i2c->activity == 0) && (i2c->tx_status == 0x1F))) {} // Be sure the sending is over
    i2c->onbus = 0;
#if defined(FRAMEWORK_LOG_ENABLED)
    printf ("End of transmission\n");
#endif

    return SUCCESS;
}



int8_t i2c_read(i2c_handle_t* handle, uint8_t to, uint8_t* payload, int length) {
    int i;
#if defined(FRAMEWORK_LOG_ENABLED)
    printf ("Start of reception\n");
#endif
    while (i2c->activity == 1) {}
    i2c->onbus = 1;
    /* Prepare the buffer with only the address */
    addr_buf = (1 << 8) | (to << 1) | 0x1; // reads slave
    while  ((i2c->tx_status & 0x1) != 1){} // As long as the fifo is full
    i2c->tx_data = addr_buf;  
    for (i = 0; i < length; i++) 
    {
	while  ((i2c->rx_status & 0x1) == 0){} // As long as the fifo is empty
	payload[i] = i2c->rx_data; 	
    }
    i2c->onbus = 0; 
   
    while (i2c->activity == 1) {} // Be sure the reception is over

    while  ((i2c->rx_status & 0x1) == 1) // Empty the rx_fifo (Last data sent by the slave 
	// but not acknowledged by the master; however it is written in the rx_fifo)
    {
       tmp = i2c->rx_data;
#if defined(FRAMEWORK_LOG_ENABLED)
       printf ("tmp %x\n", tmp);
#endif
    } 

#if defined(FRAMEWORK_LOG_ENABLED)
    /* Print the payload content */
    for (i = 0; i < length; i++) 
	printf ("Reads %x\n", payload[i]); 

    printf ("End of reception\n");
#endif

    return SUCCESS;
}



int8_t i2c_write_read(i2c_handle_t* handle, uint8_t to, uint8_t* payload, int length, uint8_t* receive, int receive_length)
{
   i2c_write(handle, to, payload, length);
   return (i2c_read(handle, to, receive, receive_length));
}
