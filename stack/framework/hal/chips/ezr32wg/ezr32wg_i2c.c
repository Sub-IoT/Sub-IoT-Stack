#include "em_device.h"
#include "em_system.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_rtc.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_i2c.h"
#include <debug.h>
#include "hwgpio.h"


#include "hwi2c.h"
#include "platform.h"

#define I2C_POLLING  100


void i2c_master_init(I2C_Init_T* init) {
	int i;
	CMU_Clock_TypeDef i2cClock;
	I2C_Init_TypeDef i2cInit;

	EFM_ASSERT(init != NULL);

	CMU_ClockEnable(cmuClock_HFPER, true);

	/* Select I2C peripheral clock */
	if (false)
	{
	#if defined( I2C0 )
	}
	else if (init->port == I2C0)
	{
	i2cClock = cmuClock_I2C0;
	#endif
	#if defined( I2C1 )
	}
	else if (init->port == I2C1)
	{
	i2cClock = cmuClock_I2C1;
	#endif
	}
	else
	{
	/* I2C clock is not defined */
	EFM_ASSERT(false);
	return;
	}
	CMU_ClockEnable(i2cClock, true);

	/* Output value must be set to 1 to not drive lines low. Set
	 SCL first, to ensure it is high before changing SDA. */
	GPIO_PinModeSet(init->sclPort, init->sclPin, gpioModeWiredAndPullUp, 1);
	GPIO_PinModeSet(init->sdaPort, init->sdaPin, gpioModeWiredAndPullUp, 1);

	/* In some situations, after a reset during an I2C transfer, the slave
	 device may be left in an unknown state. Send 9 clock pulses to
	 set slave in a defined state. */
	for (i = 0; i < 9; i++)
	{
	GPIO_PinOutSet(init->sclPort, init->sclPin);
	GPIO_PinOutClear(init->sclPort, init->sclPin);
	}

	/* Enable pins and set location */
	init->port->ROUTE = I2C_ROUTE_SDAPEN |
					  I2C_ROUTE_SCLPEN |
					  (init->portLocation << _I2C_ROUTE_LOCATION_SHIFT);

	/* Set emlib init parameters */
	i2cInit.enable = true;
	i2cInit.master = true; /* master mode only */
	i2cInit.freq = init->i2cMaxFreq;
	i2cInit.refFreq = init->i2cRefFreq;
	i2cInit.clhr = init->i2cClhr;

	I2C_Init(init->port, &i2cInit);
}

int8_t i2c_write(uint8_t address,uint8_t* tx_buffer,int length)
{
	I2C_TransferReturn_TypeDef ret;
	I2C_TransferSeq_TypeDef sensor_message =
	{
	    .addr = address,
	    .flags = I2C_FLAG_WRITE,
	    .buf[0].data = tx_buffer,
	    .buf[0].len = length,
	};

	ret = I2C_TransferInit(I2C0, &sensor_message); // start I2C write transaction with sensor
	while(ret == i2cTransferInProgress) {          // continue until all data has been sent
	    ret = I2C_Transfer(I2C0);
	}

	return ret;
}

int8_t i2c_read(uint8_t address, uint8_t* rx_buffer, int length)
{
	I2C_TransferReturn_TypeDef ret;
	I2C_TransferSeq_TypeDef sensor_message =
	{
		.addr = address,
		.flags = I2C_FLAG_READ,
		.buf[1].data = rx_buffer,
		.buf[1].len = length,
	};
	int16_t rtry = 0;
	ret = I2C_TransferInit(I2C0, &sensor_message); // start I2C write transaction with sensor
	while(ret == i2cTransferInProgress && rtry < I2C_POLLING) {          // continue until all data has been sent
		ret = I2C_Transfer(I2C0);
		rtry++;
	}

	return ret;
}


int8_t i2c_write_read(uint8_t address, uint8_t* tx_buffer,int lengthtx,uint8_t* rx_buffer,int lengthrx)
{
	I2C_TransferReturn_TypeDef ret;
	I2C_TransferSeq_TypeDef sensor_message =
	{
	    .addr = address,
	    .flags = I2C_FLAG_WRITE_READ,
	    .buf[0].data = tx_buffer,
	    .buf[0].len = lengthtx,
		.buf[1].data = rx_buffer,
		.buf[1].len = lengthrx,
	};

	ret = I2C_TransferInit(I2C0, &sensor_message); // start I2C write transaction with sensor
	while(ret == i2cTransferInProgress) {          // continue until all data has been sent
	    ret = I2C_Transfer(I2C0);
	}

	return ret;
}

