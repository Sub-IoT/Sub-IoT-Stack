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

#include "platform_sensors.h"
#include "hwgpio.h"
#include "hwadc.h"
#include "hwatomic.h"
#include "scheduler.h"
#include "em_gpio.h"
#include "em_adc.h"
#include "em_cmu.h"
#include <string.h>
#include <debug.h>
#include "em_emu.h"

#include "hwi2c.h"

#include "si7013.h"

typedef struct {
  uint32_t location;
  pin_id_t sda;
  pin_id_t scl;
} i2c_pins_t;

typedef struct i2c_handle {
  uint8_t           idx;
  I2C_TypeDef*      channel;
  CMU_Clock_TypeDef clock;
  i2c_pins_t*       pins;
} i2c_handle_t;

//static uint32_t temp_offset = 0;
static i2c_handle_t* i2c = NULL;

/** Flag used to indicate ADC is finished */
//static volatile bool adcConversionComplete = false;

void initSensors()
{
		//Humidity/temp
	CMU_ClockEnable(cmuClock_GPIO, true);
	/* Enable si7021 sensor isolation switch */
	GPIO_PinModeSet(gpioPortF, 8, gpioModePushPull, 1);

  i2c = i2c_init(0, 1, 10000, true);
	bool si7013_status = Si7013_Detect((I2C_TypeDef*) (i2c->channel), SI7021_ADDR, NULL);
}


void getHumidityAndTemperature(uint32_t *rhData, int32_t *tData)
{
	Si7013_MeasureRHAndTemp((I2C_TypeDef*) (i2c->channel), SI7021_ADDR, rhData, tData);
}


// ADC IRQ
//void ADC0_IRQHandler(void)
//{
//   uint32_t flags;
//
//   /* Clear interrupt flags */
//   flags = ADC_IntGet( ADC0 );
//   ADC_IntClear( ADC0, flags );
//
//   adcConversionComplete = true;
//}

