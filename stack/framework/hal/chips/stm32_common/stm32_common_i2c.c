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
#include <stdio.h>


//#include "hwgpio.h"
#include "hwi2c.h"

#include "platform.h"
#include "ports.h"
#include "stm32_device.h"
#include "stm32_common_gpio.h"

#include "log.h"

// TODO use other ways to avoid long polling
#define I2C_POLLING  100000
#define I2C_DEFAULT_TIMEOUT	1000

#ifndef IC2_BAUDRATE
#define IC2_BAUDRATE 100000
#endif

//HAL_MAX_DELAY

#if defined(STM32L0) // TODO not supported yet on STM32L1 (CubeMX HAL I2C API is not compatible)

#if (I2C_COUNT > 0)

#define I2C_CLK_ENABLE(i2c_instance)                          \
do {                                                          \
    switch(i2c_instance)                                      \
    {                                                         \
      case I2C1_BASE: __HAL_RCC_I2C1_CLK_ENABLE(); break;     \
      case I2C2_BASE: __HAL_RCC_I2C2_CLK_ENABLE(); break;     \
      case I2C3_BASE: __HAL_RCC_I2C3_CLK_ENABLE(); break;     \
      default: assert(false);                                 \
    }                                                         \
  } while(0)

#define I2C_CLK_DISABLE(i2c_instance)                         \
do {                                                          \
    switch(i2c_instance)                                      \
    {                                                         \
      case I2C1_BASE: __HAL_RCC_I2C1_CLK_DISABLE(); break;    \
      case I2C2_BASE: __HAL_RCC_I2C2_CLK_DISABLE(); break;    \
      case I2C3_BASE: __HAL_RCC_I2C3_CLK_DISABLE(); break;     \
      default: assert(false);                                 \
    }                                                         \
  } while(0)

typedef struct {
  pin_id_t sda;
  pin_id_t scl;
} i2c_pins_t;

// TODO to be completed with all documented locations

/*
const PinMap PinMap_I2C_SDA[] = {
{PA_10, I2C_1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF6_I2C1)}, // ARDUINO D2
{PB_4,  I2C_3, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF7_I2C3)}, // Pin not available on any connector
{PB_7,  I2C_1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF1_I2C1)}, // ARDUINO D5 - Used also to drive LED4
{PB_9,  I2C_1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)}, // ARDUINO D14
{PB_11, I2C_2, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF6_I2C2)}, // Pin not available on any connector
{PB_14, I2C_2, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF5_I2C2)}, // ARDUINO D12
{PC_1,  I2C_3, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF7_I2C3)}, // Pin not available on any connector
{NC,    NC,    0}
};

const PinMap PinMap_I2C_SCL[] = {
{PA_8,  I2C_3, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF7_I2C3)}, // ARDUINO D7
{PA_9,  I2C_1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF6_I2C1)}, // ARDUINO D8
{PB_6,  I2C_1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF1_I2C1)}, // ARDUINO D10 - Used also to drive LED3
{PB_8,  I2C_1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)}, // ARDUINO D15
{PB_10, I2C_2, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF6_I2C2)}, // Pin not available on any connector
{PB_13, I2C_2, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF5_I2C2)}, // ARDUINO D13
{PC_0,  I2C_3, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF7_I2C3)}, // Pin not available on any connector
{NC,    NC,    0}
};
*/

//#define I2C_TIMINGR_STANDARD_PCLK_4M  0
//#define I2C_TIMINGR_STANDARD_PCLK_2M  1

#define I2C_TIMINGR_INVALID_VAL  0x00000000

typedef struct{
    uint32_t clk_32M;
    uint32_t clk_16M;
    uint32_t clk_4M;
    uint32_t clk_2M;

} i2c_timingr_value_struct_t;

typedef  struct{
    i2c_timingr_value_struct_t i2c_standard_speed;
    i2c_timingr_value_struct_t i2c_high_speed;
} i2c_speed_struc_t;

const i2c_speed_struc_t I2C_TIMINGR_LUT = {
    .i2c_standard_speed = {
        .clk_32M = 0x00B07DB9,
        .clk_16M = 0x00503D5A,
        .clk_4M = 0x00201111,
        .clk_2M =  0x00100608
    },
    .i2c_high_speed = {
        .clk_32M = 0x00900F2E,
        .clk_16M = 0x00300619,
        .clk_4M = I2C_TIMINGR_INVALID_VAL,
        .clk_2M = I2C_TIMINGR_INVALID_VAL
    }
};

//const uint32_t I2C_TIMINGR_LUT[] = {0x00201111, 0x00100608};

typedef struct i2c_handle {
  I2C_HandleTypeDef hal_handle;
} i2c_handle_t;

static i2c_handle_t handle[I2C_COUNT] = {
  {
    .hal_handle.Instance = NULL,
  }
};

static inline uint32_t get_i2c_timing(int hz)
{
    uint32_t tim = 0;
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    i2c_timingr_value_struct_t timing_struct;

    //assert(hz == 100000); //other speeds not supported with MSI RC

    switch(hz)
    {
      case 100000:
        timing_struct = I2C_TIMINGR_LUT.i2c_standard_speed;
        break;
      case 400000:
        timing_struct = I2C_TIMINGR_LUT.i2c_high_speed;
        break;
      default:
        assert(false);
    }
    switch(pclk1/1000000)
    {
      case 2:
        tim = timing_struct.clk_2M;
        break;
      case 4:
        tim =timing_struct.clk_4M;
        break;
      case 16:
        tim = timing_struct.clk_16M;
        break;
      case 32:
        tim = timing_struct.clk_32M;
        break;
      default:
        assert(false);
    }

//    switch(pclk1/1000000){
//        case 2:
//            tim = I2C_TIMINGR_LUT[I2C_TIMINGR_STANDARD_PCLK_2M];
//            break;
//        case 4:
//            tim = I2C_TIMINGR_LUT[I2C_TIMINGR_STANDARD_PCLK_4M];
//            break;
//        default:
//            assert(false); // other MSI RC do not support 100kHz i2c;
//            break;
//    }
    /*
    switch (hz) {
        case 100000:
            tim = 0x00000004; // Standard mode with Rise Time = 400ns and Fall Time = 100ns
            break;
        case 400000:
            tim = 0x00901850; // Fast mode with Rise Time = 250ns and Fall Time = 100ns
            break;
        case 1000000:
            tim = 0x00700818; // Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
            break;
        default:
            break;
    }*/
    assert(tim != I2C_TIMINGR_INVALID_VAL);
    return tim;
}

i2c_handle_t* i2c_init(uint8_t idx, uint8_t pins, uint32_t baudrate, bool pullup)
{
  assert(pins==0);
  assert(idx < I2C_COUNT);

  GPIO_InitTypeDef gpio_init_options;

  I2C_CLK_ENABLE((uint32_t)(i2c_ports[idx].i2c));

  // Make sure no slave is pulling the SDA port low
  // first init SDA as input
  gpio_init_options.Mode = GPIO_MODE_INPUT;
  gpio_init_options.Pull = GPIO_NOPULL;
  gpio_init_options.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  hw_gpio_configure_pin_stm(i2c_ports[idx].sda_pin, &gpio_init_options);

  // SCL as output
  gpio_init_options.Mode = GPIO_MODE_OUTPUT_PP;
  hw_gpio_configure_pin_stm(i2c_ports[idx].scl_pin, &gpio_init_options);

  // if SDA is pulled low, clock the SCK to free it
  uint8_t i = 0;
  while(!hw_gpio_get_in(i2c_ports[idx].sda_pin) && i < 255)
  {
	  hw_gpio_set(i2c_ports[idx].scl_pin);
	  hw_busy_wait(10);
	  hw_gpio_clr(i2c_ports[idx].scl_pin);
	  hw_busy_wait(10);
    i++;
  }

  // then configure the I2C port as usual
  gpio_init_options.Alternate = i2c_ports[idx].scl_alternate;
  gpio_init_options.Mode = GPIO_MODE_AF_OD;
  if(pullup)
    gpio_init_options.Pull = GPIO_PULLUP;
  else
    gpio_init_options.Pull = GPIO_NOPULL;
  gpio_init_options.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  
  hw_gpio_configure_pin_stm(i2c_ports[idx].scl_pin, &gpio_init_options);
  gpio_init_options.Alternate = i2c_ports[idx].sda_alternate;
  hw_gpio_configure_pin_stm(i2c_ports[idx].sda_pin, &gpio_init_options);

  I2C_InitTypeDef i2c_init_options;
  i2c_init_options.Timing = get_i2c_timing(baudrate);
  i2c_init_options.OwnAddress1 = 0;
  i2c_init_options.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  i2c_init_options.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  i2c_init_options.OwnAddress2 = 0;
  i2c_init_options.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  i2c_init_options.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  handle[idx].hal_handle.Init = i2c_init_options;
  handle[idx].hal_handle.Instance = i2c_ports[idx].i2c;
  if (HAL_I2C_Init(&handle[idx].hal_handle) != HAL_OK)
  {
    return NULL;
  }

  I2C_CLK_DISABLE((uint32_t)(i2c_ports[idx].i2c));

  return &handle[idx];
}

int8_t i2c_deinit(i2c_handle_t* i2c)
{
  HAL_StatusTypeDef status = HAL_I2C_DeInit(&i2c->hal_handle);
  I2C_CLK_DISABLE((uint32_t)i2c->hal_handle.Instance);
  return status == HAL_OK;
}

void i2c_acquire(i2c_handle_t* i2c)
{
  // TODO assuming no mutual access for now, so no locking
  I2C_CLK_ENABLE((uint32_t)i2c->hal_handle.Instance);
}

void i2c_release(i2c_handle_t* i2c)
{
  I2C_CLK_DISABLE((uint32_t)i2c->hal_handle.Instance);
}

int8_t i2c_write(i2c_handle_t* i2c, uint8_t to, uint8_t* payload, int length) {
  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&i2c->hal_handle, (uint16_t) to << 1, payload, length, I2C_DEFAULT_TIMEOUT);
  return status == HAL_OK;
}

int8_t i2c_read(i2c_handle_t* i2c, uint8_t to, uint8_t* payload, int length) {

  HAL_StatusTypeDef status = HAL_I2C_Master_Receive(&i2c->hal_handle, (uint16_t) to << 1, payload, length, I2C_DEFAULT_TIMEOUT);
  return status == HAL_OK;
}

int8_t i2c_read_memory(i2c_handle_t* i2c, uint8_t to, uint16_t register_address,  uint8_t register_address_size, uint8_t* payload, int length)
{
  uint8_t mem_addr_size = register_address_size == 8 ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT;
  int status = HAL_I2C_Mem_Read(&i2c->hal_handle, (uint16_t) to << 1, register_address, mem_addr_size, payload, length, I2C_DEFAULT_TIMEOUT);
  return status == HAL_OK;
}

int8_t i2c_write_memory(i2c_handle_t* i2c, uint8_t to, uint16_t register_address,  uint8_t register_address_size, uint8_t* payload, int length)
{
  uint8_t mem_addr_size = register_address_size == 8 ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT;
  int status = HAL_I2C_Mem_Write(&i2c->hal_handle, (uint16_t) to << 1, register_address, mem_addr_size, payload, length, I2C_DEFAULT_TIMEOUT);
  return status == HAL_OK;
}

int8_t i2c_write_read(i2c_handle_t* i2c, uint8_t to, uint8_t* payload, int length, uint8_t* receive, int receive_length)
{
  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&i2c->hal_handle, (uint16_t) to << 1, payload, length, I2C_DEFAULT_TIMEOUT);
  if (status != HAL_OK) return 0;

  status = HAL_I2C_Master_Receive(&i2c->hal_handle, (uint16_t) to << 1, receive, receive_length, I2C_DEFAULT_TIMEOUT);
  return status == HAL_OK;
}

#endif // I2C_COUNT

#endif
