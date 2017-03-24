#include <debug.h>
#include <stdio.h>


#include "hwgpio.h"
#include "hwi2c.h"

#include "platform.h"

#include "stm32l1xx_hal.h"

// TODO use other ways to avoid long polling
#define I2C_POLLING  100000
#define I2C_DEFAULT_TIMEOUT	HAL_MAX_DELAY

#define I2CS       3

typedef struct {
  pin_id_t sda;
  pin_id_t scl;
} i2c_pins_t;

#define UNDEFINED_LOCATION  \
  .scl      = { .port = 0,         .pin =  0 },   \
  .sda      = { .port = 0,         .pin =  0 }

// TODO to be completed with all documented locations
static i2c_pins_t location[I2CS]= {
  {
		  UNDEFINED_LOCATION
  },
  {
    // I2C 1
	.scl      = { .port = 1, .pin =  6 },
    .sda      = { .port = 1, .pin =  7 }
  },
  {
  		  UNDEFINED_LOCATION
   }
};

typedef struct i2c_handle {
  uint8_t           idx;
  I2C_HandleTypeDef handle;
  uint32_t pins;
  GPIO_TypeDef* port;
  uint32_t alternate;
} i2c_handle_t;

static i2c_handle_t handle[I2CS] = {
  {
    .idx     = 0,
  },
  {
    .idx     = 1,
	.handle.Instance = I2C1,
	.pins = GPIO_PIN_7|GPIO_PIN_6,
    .port = GPIOB,
    .alternate = GPIO_AF4_I2C1
  } ,
  {
	    .idx     = 2,
		.handle = I2C2,
		.pins = GPIO_PIN_10|GPIO_PIN_11,
	    .port = GPIOB,
	    .alternate = GPIO_AF4_I2C2
	  }
};

i2c_handle_t* i2c_init(uint8_t idx, uint8_t pins)
{
	assert (pins==0);
	assert(idx > 0 && idx < 3);

	GPIO_InitTypeDef GPIO_InitStruct;

	if (idx == 1)
	{
		__HAL_RCC_I2C1_CLK_ENABLE();
	} else if (idx == 2) {
		__HAL_RCC_I2C2_CLK_ENABLE();
	}

	GPIO_InitStruct.Pin = handle[idx].pins;
	GPIO_InitStruct.Alternate = handle[idx].alternate;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(handle[idx].port, &GPIO_InitStruct);

	handle[idx].handle.Init.ClockSpeed = 100000;
	handle[idx].handle.Init.DutyCycle = I2C_DUTYCYCLE_2;
	handle[idx].handle.Init.OwnAddress1 = 0;
	handle[idx].handle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	handle[idx].handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	handle[idx].handle.Init.OwnAddress2 = 0;
	handle[idx].handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	handle[idx].handle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&handle[idx].handle) != HAL_OK)
	{
		return NULL;
	}

//	CMU_ClockEnable(cmuClock_HFPER, true);
//	CMU_ClockEnable(handle[idx].clock, true);
//
//  handle[idx].pins = &location[idx][pins];
//
//	// Output value must be set to 1 to not drive lines low.
//	// Set SCL first, to ensure it is high before changing SDA.
//  assert(hw_gpio_configure_pin(handle[idx].pins->scl, false, gpioModeWiredAndPullUp, 1) == SUCCESS);
//  assert(hw_gpio_configure_pin(handle[idx].pins->sda, false, gpioModeWiredAndPullUp, 1) == SUCCESS);
//
//	// In some situations, after a reset during an I2C transfer, the slave
//	// device may be left in an unknown state. Send 9 clock pulses to
//	// set slave in a defined state.
//	for(uint8_t i = 0; i < 9; i++)	{
//    hw_gpio_set(handle[idx].pins->scl);
//    hw_gpio_clr(handle[idx].pins->scl);
//	}
//
//	// enable pins and set location
//	handle[idx].channel->ROUTE = I2C_ROUTE_SDAPEN
//                             | I2C_ROUTE_SCLPEN
//                             | handle[idx].pins->location;
//
//  I2C_Init_TypeDef i2cInit = {
//    .enable  = true,
//    .master  = true,
//    .freq    = I2C_FREQ_STANDARD_MAX,   // set to standard rate
//    .refFreq = 0,                       // currently configured clock
//    .clhr    = i2cClockHLRStandard      // Set to use 4:4 low/high duty cycle
//  };
//
//	I2C_Init(handle[idx].channel, &i2cInit);
  
  return &handle[idx];
}

int8_t i2c_write(i2c_handle_t* i2c, uint8_t to, uint8_t* payload, int length) {
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&i2c->handle, (uint16_t) to << 1, payload, length, I2C_DEFAULT_TIMEOUT);
	return status == HAL_OK;
}

int8_t i2c_read(i2c_handle_t* i2c, uint8_t to, uint8_t* payload, int length) {

	HAL_StatusTypeDef status = HAL_I2C_Master_Receive(&i2c->handle, (uint16_t) to << 1, payload, length, I2C_DEFAULT_TIMEOUT);
	return status == HAL_OK;
}

int8_t i2c_read_memory(i2c_handle_t* i2c, uint8_t to, uint16_t register_address,  uint8_t register_address_size, uint8_t* payload, int length)
{
	uint8_t mem_addr_size = register_address_size == 8 ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT;
	int status = HAL_I2C_Mem_Read(&i2c->handle, (uint16_t) to << 1, register_address, mem_addr_size, payload, length, I2C_DEFAULT_TIMEOUT);
	return status == HAL_OK;
}

int8_t i2c_write_memory(i2c_handle_t* i2c, uint8_t to, uint16_t register_address,  uint8_t register_address_size, uint8_t* payload, int length)
{
	uint8_t mem_addr_size = register_address_size == 8 ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT;
	int status = HAL_I2C_Mem_Write(&i2c->handle, (uint16_t) to << 1, register_address, mem_addr_size, payload, length, I2C_DEFAULT_TIMEOUT);
	return status == HAL_OK;
}

int8_t i2c_write_read(i2c_handle_t* i2c, uint8_t to, uint8_t* payload, int length, uint8_t* receive, int receive_length)
{
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&i2c->handle, (uint16_t) to << 1, payload, length, I2C_DEFAULT_TIMEOUT);
	if (status != HAL_OK) return 0;

	status = HAL_I2C_Master_Receive(&i2c->handle, (uint16_t) to << 1, receive, receive_length, I2C_DEFAULT_TIMEOUT);
	return status == HAL_OK;
}
