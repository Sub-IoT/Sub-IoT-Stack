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

static uint32_t temp_offset = 0;
static i2c_handle_t* i2c = NULL;

/** Flag used to indicate ADC is finished */
static volatile bool adcConversionComplete = false;

void initSensors()
{
	// ADC init for battery monitor
	adc_init(adcReference1V25, adcReferenceVDDDiv3, 100);

	/* Manually set some calibration values */
	ADC0->CAL = (0x7C << _ADC_CAL_SINGLEOFFSET_SHIFT) | (0x1F << _ADC_CAL_SINGLEGAIN_SHIFT);


	//Humidity/temp
	CMU_ClockEnable(cmuClock_GPIO, true);
	/* Enable si7021 sensor isolation switch */
	GPIO_PinModeSet(gpioPortF, 8, gpioModePushPull, 1);

	i2c = i2c_init(0, 1);
	bool si7013_status = Si7013_Detect((I2C_TypeDef*) (i2c->channel), SI7021_ADDR, NULL);
}


//Internal Temp Chip
 void internalTempSensor_init(void)
{
	//adc_calibrate();

	// Initialises ADC
	adc_init(adcReference1V25, adcInputSingleTemp, 100);


	/* This is a work around for Chip Rev.D Errata, Revision 0.6. */
	/* Check for product revision 16 and 17 and set the offset */
	/* for ADC0_TEMP_0_READ_1V25. */
	uint8_t prod_rev = (DEVINFO->PART & _DEVINFO_PART_PROD_REV_MASK) >> _DEVINFO_PART_PROD_REV_SHIFT;

	if( (prod_rev == 16) || (prod_rev == 17) )
	{
		temp_offset = 112;
	}
	else
	{
		temp_offset = 0;
	}
}

float tempsensor_read_celcius()
{
	//todo: take into account warmup time
	adc_start();

	/* Wait in EM1 for ADC to complete */
	while (!adcConversionComplete) EMU_EnterEM1();
	adc_clear_interrupt();

	/* Read sensor value */
	/* According to rev. D errata ADC0_TEMP_0_READ_1V25 should be decreased */
	/* by the offset  but it is the same if ADC reading is increased - */
	/* reference manual 28.3.4.2. */
	uint32_t temp = adc_get_value() + temp_offset;
	return convertAdcToCelsius(temp);
}

float convertAdcToCelsius(int32_t adcSample)
{
  float temp;
  /* Factory calibration temperature from device information page. */
  float cal_temp_0 = (float)((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK)
                             >> _DEVINFO_CAL_TEMP_SHIFT);

  float cal_value_0 = (float)((DEVINFO->ADC0CAL2
                               & _DEVINFO_ADC0CAL2_TEMP1V25_MASK)
                              >> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);

  /* Temperature gradient (from datasheet) */
  float t_grad = -6.27;

  temp = (cal_temp_0 - ((cal_value_0 - adcSample)  / t_grad));

  return temp;
}

//Battery
uint32_t getBattery(void)
{
  uint32_t vData;
  /* Sample ADC */
  adcConversionComplete = false;
  ADC_Start(ADC0, adcStartSingle);
  while (!adcConversionComplete) EMU_EnterEM1();
  vData = ADC_DataSingleGet( ADC0 );

  vData = 3 * 1250 * (vData / 4095.0);
  return vData;
}

void getHumidityAndTemperature(uint32_t *rhData, int32_t *tData)
{
	Si7013_MeasureRHAndTemp((I2C_TypeDef*) (i2c->channel), SI7021_ADDR, rhData, tData);
}


// ADC IRQ
void ADC0_IRQHandler(void)
{
   uint32_t flags;

   /* Clear interrupt flags */
   flags = ADC_IntGet( ADC0 );
   ADC_IntClear( ADC0, flags );

   adcConversionComplete = true;
}

