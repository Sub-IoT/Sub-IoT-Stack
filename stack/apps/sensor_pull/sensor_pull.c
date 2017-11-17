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


// This examples does not push sensor data to gateway(s) continuously, but instead writes the sensor value to a local file,
// which can then be fetched on request.
// Contrary to the sensor_push example we are now defining an Access Profile which has a periodic scan automation enabled.
// The sensor will sniff the channel every second for background adhoc synchronization frames, to be able to receive requests from other nodes.

#include "hwleds.h"
#include "hwsystem.h"
#include "scheduler.h"
#include "timer.h"
#include "assert.h"

#include <stdio.h>
#include <stdlib.h>

#include "hwlcd.h"
#include "d7ap_stack.h"
#include "fs.h"
#include "log.h"
#include "compress.h"

#if !defined(USE_SX127X) && !defined(USE_NETDEV_SX127X)
  #error "background frames are only supported by the sx127x driver for now"
#endif

#if (defined PLATFORM_EFM32GG_STK3700 || defined PLATFORM_EZR32LG_WSTK6200A)
  #include "platform_sensors.h"
#endif

#if (defined PLATFORM_B_L072Z_LRWAN1)
	#include "led.h"
	#define LED_FLASH_GREEN()	led_flash_green()
#else
	#define LED_FLASH_GREEN()
#endif

#ifdef HAS_LCD
  #include "platform_lcd.h"
  #define LCD_WRITE_STRING(...) lcd_write_string(__VA_ARGS__)
  #ifdef PLATFORM_EFM32GG_STK3700
    // STK3700 LCD does not use multiple lines
    #define LCD_WRITE_LINE(line, ...) lcd_write_string(__VA_ARGS__)
  #else
    #define LCD_WRITE_LINE(line, ...) lcd_write_line(line, __VA_ARGS__)
  #endif
#else
  #define LCD_WRITE_STRING(...)
  #define LCD_WRITE_LINE(...)
#endif


#define SENSOR_FILE_ID           0x40
#define SENSOR_FILE_SIZE         8
#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC * 10

void execute_sensor_measurement()
{
  // first get the sensor reading ...

  uint8_t sensor_values[SENSOR_FILE_SIZE] = { 0 };

#if (defined PLATFORM_EZR32LG_WSTK6200A || defined PLATFORM_EFM32GG_STK3700 || defined PLATFORM_EZR32LG_USB01)
  char str[30];

  float internal_temp = hw_get_internal_temperature();
#if (defined PLATFORM_EFM32GG_STK3700)
  lcd_write_temperature(internal_temp*10, 1);
#else
  sprintf(str, "Int T: %2d.%d C", (int)internal_temp, (int)(internal_temp*10)%10);
  LCD_WRITE_LINE(2,str);
#endif

  log_print_string(str);

  uint32_t rhData = 0;
  uint32_t tData = 0;
#if ! defined PLATFORM_EZR32LG_USB01 && ! defined PLATFORM_EFM32GG_STK3700
  getHumidityAndTemperature(&rhData, &tData);

  sprintf(str, "Ext T: %d.%d C", (tData/1000), (tData%1000)/100);
  LCD_WRITE_LINE(3,str);
  log_print_string(str);

  sprintf(str, "Ext H: %d.%d", (rhData/1000), (rhData%1000)/100);
  LCD_WRITE_LINE(4,str);
  log_print_string(str);
#endif

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
  // We will write to a local file

  uint8_t alp_command[4 + SENSOR_FILE_SIZE] = {
    // ALP Control byte
    ALP_OP_WRITE_FILE_DATA,
    // File Data Request operand:
    SENSOR_FILE_ID, // the file ID
    0, // offset in file
    SENSOR_FILE_SIZE // data length
    // the sensor data, see below
  };

  memcpy(alp_command + 4, sensor_values, SENSOR_FILE_SIZE);

  uint8_t resp = 0;
  alp_process_command(alp_command, sizeof(alp_command), alp_command, &resp, ALP_CMD_ORIGIN_APP);

  timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);

  LED_FLASH_GREEN();
}

void init_user_files()
{
  // file 0x40: contains our sensor data
  fs_file_header_t file_header = (fs_file_header_t){
      .file_properties.action_protocol_enabled = 0,
      .file_properties.permissions = 0, // TODO
      .length = SENSOR_FILE_SIZE
  };

  fs_init_file(SENSOR_FILE_ID, &file_header, NULL);
}

void bootstrap()
{
    log_print_string("Device booted\n");

    dae_access_profile_t access_classes[2] = {
        {
            // AC used for pushing data to the GW, no scanning
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
                .cca = 86,
                .duty = 0,
            }
        },
        {
            // AC used for scanning for BG request every second
            .channel_header = {
                .ch_coding = PHY_CODING_PN9,
                .ch_class = PHY_CLASS_NORMAL_RATE,
                .ch_freq_band = PHY_BAND_868
            },
            .subprofiles[0] = {
              .subband_bitmap = 0x01,
              .scan_automation_period = compress_data(1024, true),
            },
            .subbands[0] = (subband_t){
                .channel_index_start = 0,
                .channel_index_end = 0,
                .eirp = 10,
                .cca = 86,
                .duty = 0,
            }
        }
    };

    fs_init_args_t fs_init_args = (fs_init_args_t){
        .access_profiles_count = 2,
        .access_profiles = access_classes,
        .access_class = 0x11, // use scanning AC
        .fs_user_files_init_cb = &init_user_files
    };

    d7ap_stack_init(&fs_init_args, NULL, false, NULL);

#if (defined PLATFORM_EFM32GG_STK3700 || defined PLATFORM_EZR32LG_WSTK6200A)
    initSensors();
#endif

    sched_register_task(&execute_sensor_measurement, NULL);
    timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);

    LCD_WRITE_STRING("Sensor push\n");
}
