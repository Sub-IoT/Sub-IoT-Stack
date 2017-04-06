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

// This examples pushes sensor data to gateway(s) by writing it to a local file, which is configured to trigger a file action (using D7AActP)
// which results in reading this file and sending the result to the D7 interface. The D7 session is configured not to request ACKs.


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
#define ACTION_FILE_ID           0x41

#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC * 10


void execute_sensor_measurement()
{
#if (defined PLATFORM_EFM32HG_STK3400  || defined PLATFORM_EZR32LG_WSTK6200A \
  || defined PLATFORM_EZR32LG_OCTA || defined PLATFORM_EFM32GG_STK3700 || defined PLATFORM_EZR32LG_USB01)
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

  uint8_t sensor_values[8];
  uint16_t *pointer =  (uint16_t*) sensor_values;
  *pointer++ = (uint16_t) (internal_temp * 10);
  *pointer++ = (uint16_t) (tData /100);
  *pointer++ = (uint16_t) (rhData /100);
  *pointer++ = (uint16_t) (vdd /10);

  fs_write_file(SENSOR_FILE_ID, 0, (uint8_t*)&sensor_values,8);
#else
  // no sensor, we just write the current timestamp
  timer_tick_t t = timer_get_counter_value();
  fs_write_file(SENSOR_FILE_ID, 0, (uint8_t*)&t, sizeof(timer_tick_t));
#endif

  timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);

#ifdef PLATFORM_EZR32LG_OCTA
  led_flash_green();
#endif
}

void init_user_files()
{
    // file 0x40: contains our sensor data + configure an action file to be executed upon write
    fs_file_header_t file_header = (fs_file_header_t){
        .file_properties.action_protocol_enabled = 1,
        .file_properties.action_file_id = ACTION_FILE_ID,
        .file_properties.action_condition = ALP_ACT_COND_WRITE,
        .file_properties.storage_class = FS_STORAGE_VOLATILE,
        .file_properties.permissions = 0, // TODO
        .length = SENSOR_FILE_SIZE
    };

    fs_init_file(SENSOR_FILE_ID, &file_header, NULL);

    // configure file notification using D7AActP: write ALP command to broadcast changes made to file 0x40 in file 0x41

    // first generate ALP command. We do this manually for now (until we have an API for this).
    // Please refer to the spec for the format

    uint8_t alp_command[4] = {
      // ALP Control byte
      ALP_OP_READ_FILE_DATA,
      // File Data Request operand:
      SENSOR_FILE_ID, // the file ID
      0, // offset in file
      SENSOR_FILE_SIZE // requested data length
    };

    // Define the D7 interface configuration used for sending the result of above ALP command on
    d7asp_master_session_config_t session_config = {
        .qos = {
            .qos_resp_mode = SESSION_RESP_MODE_NO,
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

    // finally, register D7AActP file
    fs_init_file_with_D7AActP(ACTION_FILE_ID, &session_config, alp_command, sizeof(alp_command));
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
        .fs_user_files_init_cb = &init_user_files,
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

    LCD_WRITE_STRING("EFM32 Sensor\n");
}

