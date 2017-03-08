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


// This examples pushes sensor data to gateway(s) by manually constructing an ALP command with a file read result action
// (unsolicited message)

#include "hwleds.h"
#include "hwsystem.h"
#include "scheduler.h"
#include "timer.h"
#include "assert.h"
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>

#include "hwlcd.h"
#include "hwadc.h"
#include "d7ap_stack.h"
#include "fs.h"
#include "log.h"


#include "button.h"
#if (defined PLATFORM_EFM32GG_STK3700 || defined PLATFORM_EFM32HG_STK3400 || defined PLATFORM_EZR32LG_WSTK6200A || defined PLATFORM_EZR32LG_OCTA)
  #include "platform_sensors.h"
#endif

#ifdef PLATFORM_EZR32LG_OCTA
#include "led.h"
#endif

#ifdef HAS_LCD
  #include "platform_lcd.h"
  #define LCD_WRITE_STRING(...) lcd_write_string(__VA_ARGS__)
  #define LCD_WRITE_LINE(line, ...) lcd_write_line(line, __VA_ARGS__)
#else
  #define LCD_WRITE_STRING(...)
  #define LCD_WRITE_LINE(...)
#endif


#define SENSOR_FILE_ID           0x40
#define SENSOR_FILE_SIZE         8
#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC * 10

// Define the D7 interface configuration used for sending the ALP command on
static d7asp_master_session_config_t session_config = {
    .qos = {
        .qos_resp_mode = SESSION_RESP_MODE_ANY,
        .qos_retry_mode = SESSION_RETRY_MODE_NO,
        .qos_stop_on_error       = false,
        .qos_record              = false
    },
    .dormant_timeout = 0,
    .addressee = {
        .ctrl = {
            .nls_method = AES_NONE,
            .id_type = ID_TYPE_NOID,
        },
        .access_class = 0x01,
        .id = 0
    }
};



void execute_sensor_measurement()
{
  // first get the sensor reading ...

  uint8_t sensor_values[SENSOR_FILE_SIZE] = { 0 };

#if (defined PLATFORM_EFM32GG_STK3700)
  float internal_temp = hw_get_internal_temperature();
  lcd_write_temperature(internal_temp*10, 1);
  uint32_t vdd = hw_get_battery();
  fs_write_file(SENSOR_FILE_ID, 0, (uint8_t*)&internal_temp, sizeof(internal_temp)); // File 0x40 is configured to use D7AActP trigger an ALP action which broadcasts this file data on Access Class 0
#elif (defined PLATFORM_EFM32HG_STK3400  || defined PLATFORM_EZR32LG_WSTK6200A || defined PLATFORM_EZR32LG_OCTA)
  char str[30];

  float internal_temp = hw_get_internal_temperature();
  sprintf(str, "Int T: %2d.%d C", (int)internal_temp, (int)(internal_temp*10)%10);
  LCD_WRITE_LINE(2,str);

  log_print_string(str);

  uint32_t rhData;
  uint32_t tData;
  getHumidityAndTemperature(&rhData, &tData);

  sprintf(str, "Ext T: %d.%d C", (tData/1000), (tData%1000)/100);
  LCD_WRITE_LINE(3,str);
  log_print_string(str);

  sprintf(str, "Ext H: %d.%d", (rhData/1000), (rhData%1000)/100);
  LCD_WRITE_LINE(4,str);
  log_print_string(str);

  uint32_t vdd = hw_get_battery();

  sprintf(str, "Batt %d mV", vdd);
  LCD_WRITE_LINE(5,str);
  log_print_string(str);

  uint16_t *pointer =  (uint16_t*) sensor_values;
  *pointer++ = (uint16_t) (internal_temp * 10);
  *pointer++ = (uint16_t) (tData /100);
  *pointer++ = (uint16_t) (rhData /100);
  *pointer++ = (uint16_t) (vdd /10);
#else
  // no sensor, we just write the current timestamp
  timer_tick_t t = timer_get_counter_value();
  memcpy(sensor_values, (uint8_t*)&t, sizeof(timer_tick_t));
#endif

  // Generate ALP command. We do this manually for now (until we have an API for this).
  // We will be sending a return file data action, without a preceding file read request.
  // This is an unsolicited message, where we push the sensor data to the gateway(s).
  // Please refer to the spec for the format

  uint8_t alp_command[4 + SENSOR_FILE_SIZE] = {
    // ALP Control byte
    ALP_OP_RETURN_FILE_DATA,
    // File Data Request operand:
    SENSOR_FILE_ID, // the file ID
    0, // offset in file
    SENSOR_FILE_SIZE // data length
    // the sensor data, see below
  };

  memcpy(alp_command + 4, sensor_values, SENSOR_FILE_SIZE);

  alp_execute_command(alp_command, sizeof(alp_command), &session_config);
  timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);

#ifdef PLATFORM_EZR32LG_OCTA
  led_flash_green();
#endif
}

void bootstrap()
{
    log_print_string("Device booted\n");

    dae_access_profile_t access_classes[1] = {
        {
            .channel_header = {
                .ch_coding = PHY_CODING_PN9,
                .ch_class = PHY_CLASS_NORMAL_RATE,
                .ch_freq_band = PHY_BAND_868
            },
            .subprofiles[0] = {
                .subband_bitmap = 0x00, // void scan automation channel list
                .scan_automation_period = 0,
            },
            .subbands[0] = (subband_t){
                .channel_index_start = 0,
                .channel_index_end = 0,
                .eirp = 10,
                .cca = -86,
                .duty = 0,
            }
        }
    };

    fs_init_args_t fs_init_args = (fs_init_args_t){
        .access_profiles_count = 1,
        .access_profiles = access_classes,
        .access_class = 0x01
    };

    d7ap_stack_init(&fs_init_args, NULL, false, NULL);

#if (defined PLATFORM_EFM32GG_STK3700 || defined PLATFORM_EFM32HG_STK3400 || defined PLATFORM_EZR32LG_WSTK6200A || defined PLATFORM_EZR32LG_OCTA)
    initSensors();
#endif

    sched_register_task(&execute_sensor_measurement);
    timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);

    LCD_WRITE_STRING("Sensor push\n");
}

