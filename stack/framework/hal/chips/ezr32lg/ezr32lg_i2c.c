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
#include <debug.h>
#include "ezr32lg_mcu.h"
#include "em_device.h"
#include "em_system.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_rtc.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_i2c.h"

#include "hwgpio.h"
#include "hwi2c.h"

#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "errors.h"

// TODO use other ways to avoid long polling
#define I2C_POLLING  10000

#define I2CS       2
#define LOCATIONS  5

typedef struct {
  uint32_t location;
  pin_id_t sda;
  pin_id_t scl;
} i2c_pins_t;

#define UNDEFINED_LOCATION {                      \
  .location = 0,                                  \
  .scl      = PIN(0, 0),   \
  .sda      = PIN(0, 0)    \
}

// TODO to be completed with all documented locations
// TODO move to ports.h
static i2c_pins_t location[I2CS][LOCATIONS] = {
  {
    // I2C 0
    {
      .location = I2C_ROUTE_LOCATION_LOC0,
      .scl      = PIN(GPIO_PORTA, 1),
      .sda      = PIN(GPIO_PORTA, 0)
    },
    {
      .location = I2C_ROUTE_LOCATION_LOC1,
      .scl      = PIN(GPIO_PORTD, 7),
      .sda      = PIN(GPIO_PORTD, 6)
    },
    {
      .location = I2C_ROUTE_LOCATION_LOC2,
      .scl      = PIN(GPIO_PORTC, 7),
      .sda      = PIN(GPIO_PORTC, 6)
    },
    {
      .location = I2C_ROUTE_LOCATION_LOC3,
      .scl      = PIN(GPIO_PORTD, 15),
      .sda      = PIN(GPIO_PORTD, 14)
    },
    {
      .location = I2C_ROUTE_LOCATION_LOC4,
      .scl      = PIN(GPIO_PORTC, 1),
      .sda      = PIN(GPIO_PORTC, 0)
    }
  },
  {
    // I2C 1
    {
      .location = I2C_ROUTE_LOCATION_LOC0,
      .scl      = PIN(GPIO_PORTC, 5),
      .sda      = PIN(GPIO_PORTC, 4)
    },
    {
      .location = I2C_ROUTE_LOCATION_LOC1,
      .scl      = PIN(GPIO_PORTB, 12),
      .sda      = PIN(GPIO_PORTB, 11)
    },
    {
      .location = I2C_ROUTE_LOCATION_LOC2,
      .scl      = PIN(GPIO_PORTE, 1),
      .sda      = PIN(GPIO_PORTE, 0)
    },
    // no LOCATION 3
    UNDEFINED_LOCATION,
    // no LOCATION 4
    UNDEFINED_LOCATION
  }
};

typedef struct i2c_handle {
  uint8_t           idx;
  I2C_TypeDef*      channel;
  CMU_Clock_TypeDef clock;
  i2c_pins_t*       pins;
} i2c_handle_t;

static i2c_handle_t handle[I2CS] = {
  {
    .idx     = 0,
    .channel = I2C0,
    .clock   = cmuClock_I2C0
  },
  {
    .idx     = 1,
    .channel = I2C1,
    .clock   = cmuClock_I2C1
  }
};

i2c_handle_t* i2c_init(uint8_t idx, uint8_t pins, uint32_t baudrate, bool pullup) {
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(handle[idx].clock, true);

  handle[idx].pins = &location[idx][pins];

	// Output value must be set to 1 to not drive lines low.
	// Set SCL first, to ensure it is high before changing SDA.
  assert(hw_gpio_configure_pin(handle[idx].pins->scl, false, gpioModeWiredAndPullUp, 1) == SUCCESS);
  assert(hw_gpio_configure_pin(handle[idx].pins->sda, false, gpioModeWiredAndPullUp, 1) == SUCCESS);

	// In some situations, after a reset during an I2C transfer, the slave
	// device may be left in an unknown state. Send 9 clock pulses to
	// set slave in a defined state.
	for(uint8_t i = 0; i < 9; i++)	{
    hw_gpio_set(handle[idx].pins->scl);
    hw_gpio_clr(handle[idx].pins->scl);
	}

	// enable pins and set location
	handle[idx].channel->ROUTE = I2C_ROUTE_SDAPEN
                             | I2C_ROUTE_SCLPEN
                             | handle[idx].pins->location;

  I2C_Init_TypeDef i2cInit = {
    .enable  = true,
    .master  = true,
    .freq    = I2C_FREQ_STANDARD_MAX,   // set to standard rate
    .refFreq = 0,                       // currently configured clock
    .clhr    = i2cClockHLRStandard      // Set to use 4:4 low/high duty cycle
  };

	I2C_Init(handle[idx].channel, &i2cInit);

  return &handle[idx];
}

// private transfer function to handle all write, read and write/read actions
int8_t _perform_i2c_transfer(i2c_handle_t* i2c, I2C_TransferSeq_TypeDef msg) {
  int16_t rtry = 0;

   // start I2C write transaction
  I2C_TransferReturn_TypeDef ret = I2C_TransferInit(i2c->channel, &msg);

  // continue until all data has been sent
  while(ret == i2cTransferInProgress && rtry < I2C_POLLING) {
  	ret = I2C_Transfer(i2c->channel);
  	rtry++;
  }

  return ret;
}

int8_t i2c_write(i2c_handle_t* i2c, uint8_t to, uint8_t* payload, int length) {
	return _perform_i2c_transfer(i2c, (I2C_TransferSeq_TypeDef) {
    .addr        = to,
    .flags       = I2C_FLAG_WRITE,
    .buf[0].data = payload,
    .buf[0].len  = length,
	});
}

int8_t i2c_read(i2c_handle_t* i2c, uint8_t to, uint8_t* payload, int length) {
  return _perform_i2c_transfer(i2c, (I2C_TransferSeq_TypeDef) {
    .addr        = to,
    .flags       = I2C_FLAG_READ,
    .buf[0].data = payload,
    .buf[0].len  = length,
  });
}

int8_t i2c_write_read(i2c_handle_t* i2c, uint8_t to, uint8_t* payload, int length,
                      uint8_t* receive, int receive_length)
{
	return _perform_i2c_transfer(i2c, (I2C_TransferSeq_TypeDef) {
    .addr        = to,
    .flags       = I2C_FLAG_WRITE_READ,
    .buf[0].data = payload,
    .buf[0].len  = length,
	  .buf[1].data = receive,
	  .buf[1].len  = receive_length,
	});
}

int8_t i2c_read_memory(i2c_handle_t* i2c, uint8_t to, uint16_t register_address,  uint8_t register_address_size, uint8_t* payload, int length)
{
	uint8_t mem_addr_size = register_address_size == 8 ? 1 : 2;
	uint8_t send[2] = {register_address & 0xFF, register_address  >> 8};

	return _perform_i2c_transfer(i2c, (I2C_TransferSeq_TypeDef) {
	    .addr        = to,
	    .flags       = I2C_FLAG_WRITE_READ,
	    .buf[0].data = send,
	    .buf[0].len  = mem_addr_size,
		  .buf[1].data = payload,
		  .buf[1].len  = length,
		});
}

int8_t i2c_write_memory(i2c_handle_t* i2c, uint8_t to, uint16_t register_address,  uint8_t register_address_size, uint8_t* payload, int length)
{
	uint8_t mem_addr_size = register_address_size == 8 ? 1 : 2;
	uint8_t *new_payload;

	   /* Initial memory allocation */
	new_payload = (uint8_t *) malloc(length + mem_addr_size);

	if (mem_addr_size == 1)
	{
		new_payload[0] = register_address;
	} else {
		new_payload[0] = register_address & 0xFF;
		new_payload[1] = register_address >> 8;
	}

	memcpy(&new_payload[mem_addr_size], payload, length);


	return _perform_i2c_transfer(i2c, (I2C_TransferSeq_TypeDef) {
	    .addr        = to,
	    .flags       = I2C_FLAG_WRITE,
	    .buf[0].data = new_payload,
	    .buf[0].len  = length + mem_addr_size,
		});
}
