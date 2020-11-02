/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication using Neo M8N GPS
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
 *
 * \author	Liam Wickins <liamw9534@gmail.com>
 *
 */


#include <stdio.h>
#include <stdlib.h>

#include "hwleds.h"
#include "hwsystem.h"
#include "hwlcd.h"

#include "scheduler.h"
#include "timer.h"
#include "debug.h"
#include "d7ap_fs.h"
#include "log.h"
#include "compress.h"

#include "d7ap.h"
#include "alp_layer.h"
#include "dae.h"
#include "platform_defs.h"
#include "modules_defs.h"
#include "platform.h"

#ifdef PLATFORM_USE_NEO_M8N_GPS
#include "gps.h"
#endif

#ifdef MODULE_LORAWAN
  #error "sensor_pull app is not compatible with LoRaWAN, so disable MODULE_LORAWAN in cmake"
#endif

//#if !defined(USE_SX127X) && !defined(USE_NETDEV_DRIVER)
//  #error "background frames are only supported by the sx127x driver for now"
//#endif


#define GPS_POS_FILE_ID		      0x40
#define GPS_ENABLE_FILE_ID            0x41
#define GPS_DISABLE_FILE_ID           0x42
#define GPS_REPORTING_PERIOD_FILE_ID  0x43


// LEDs used to indicate device status
#define LED_COMMAND_PENDING      0
#define LED_COMMAND_RECEIVED     1
#define LED_GPS_TRACKING	 2
#define LED_GPS_FIXED            3


static uint32_t gps_reporting_period = 10;


#if PLATFORM_NUM_LEDS > 0
  #include "hwleds.h"

  void led_blink_off()
  {
    led_off(LED_COMMAND_RECEIVED);
  }

  void led_blink()
  {
    led_on(LED_COMMAND_RECEIVED);

    timer_post_task_delay(&led_blink_off, TIMER_TICKS_PER_SEC * 1.0);
  }
#endif


static alp_interface_config_d7ap_t itf_config = (alp_interface_config_d7ap_t){
    .itf_id = ALP_ITF_ID_D7ASP,
    .d7ap_session_config = {
      .qos = {
          .qos_resp_mode = SESSION_RESP_MODE_ANY,
          .qos_retry_mode = SESSION_RETRY_MODE_NO
      },
      .dormant_timeout = 0,
      .addressee = {
          .ctrl = {
              .nls_method = AES_NONE,
              .id_type = ID_TYPE_NBID,
          },
          .access_class = 0x01,
          .id = { }
      }
    }
};


void on_alp_command_completed_cb(uint8_t tag_id, bool success)
{
    if(success)
      printf("\nCommand (%i) completed successfully\n", tag_id);
    else
      printf("\nCommand failed, no ack received\n");
#if PLATFORM_NUM_LEDS > 0
    led_off(LED_COMMAND_PENDING);
#endif
}

void on_alp_command_result_cb(alp_command_t *alp_command, alp_interface_status_t* origin_itf_status)
{
  if(origin_itf_status && (origin_itf_status->itf_id == ALP_ITF_ID_D7ASP) && (origin_itf_status->len > 0)) {
      d7ap_session_result_t* d7_result = ((d7ap_session_result_t*)origin_itf_status->itf_status);
      printf("\nrecv response @ %i dB link budget\n", d7_result->rx_level);
  }
  printf("\nresponse payload length: %u\n", fifo_get_size(&alp_command->alp_command_fifo));
  fifo_skip(&alp_command->alp_command_fifo, fifo_get_size(&alp_command->alp_command_fifo));
}

static alp_status_codes_t alp_unhandled_read_action_cb(const alp_interface_status_t* origin_itf_status, alp_operand_file_data_request_t operand, uint8_t* alp_response)
{
	printf("\nalp_unhandled_read_action_cb file_id=%02x offset=%u length=%u\n", operand.file_offset.file_id, operand.file_offset.offset, operand.requested_data_length);
	return ALP_STATUS_FILE_ID_NOT_EXISTS;
}

static alp_status_codes_t alp_unhandled_write_action_cb(const alp_interface_status_t* origin_itf_status, alp_operand_file_data_t *operand)
{
#if PLATFORM_NUM_LEDS > 0
	led_blink();
#endif
	if (operand->file_offset.file_id == GPS_ENABLE_FILE_ID)
	{
#if PLATFORM_NUM_LEDS > 0
	    led_on(LED_GPS_TRACKING);
#endif
		printf("\ngps_enable\n");
#ifdef PLATFORM_USE_NEO_M8N_GPS
		gps_enable();
#endif
	}
	else if (operand->file_offset.file_id == GPS_DISABLE_FILE_ID)
	{
#if PLATFORM_NUM_LEDS > 0
	    led_off(LED_GPS_TRACKING);
	    led_off(LED_GPS_FIXED);
#endif
		printf("\ngps_disable\n");
#ifdef PLATFORM_USE_NEO_M8N_GPS
		gps_disable();
#endif
	}
	else if (operand->file_offset.file_id == GPS_REPORTING_PERIOD_FILE_ID)
	{
		gps_reporting_period = *(uint32_t *)operand->data;
#ifdef PLATFORM_USE_NEO_M8N_GPS
		printf("\ngps_reporting_period = %u\n", gps_reporting_period);
#endif
	}
	else
	{
		printf("\nfile_id not supported\n");
		return ALP_STATUS_FILE_ID_NOT_EXISTS;
	}
	return ALP_STATUS_OK;
}

static alp_init_args_t alp_init_args = {
	.alp_command_completed_cb = on_alp_command_completed_cb,
	.alp_command_result_cb = on_alp_command_result_cb,
	.alp_unhandled_read_action_cb = alp_unhandled_read_action_cb,
	.alp_unhandled_write_action_cb = alp_unhandled_write_action_cb
};


#ifdef PLATFORM_USE_NEO_M8N_GPS

void gps_event_handler(gps_event_t *event)
{
	static unsigned int cnt = 0;

#if PLATFORM_NUM_LEDS > 0
	if (event->pos.is_fixed)
		led_on(LED_GPS_FIXED);
	else
		led_off(LED_GPS_FIXED);
#endif

	if ((cnt++ % gps_reporting_period) == 0)
	{
		printf("\n*GPS_POS itow=%u fixed=%u ttff=%u lon=%d lat=%d\n", event->pos.itow, event->pos.is_fixed, event->pos.ttff, event->pos.longitude, event->pos.latitude);
		alp_command_t *cmd;
		cmd = alp_layer_command_alloc(false, false);
		alp_append_forward_action(cmd, (alp_interface_config_t*)&itf_config, sizeof(itf_config));
		alp_append_return_file_data_action(cmd, GPS_POS_FILE_ID, 0, sizeof(gps_event_t), (uint8_t *)event);
		alp_layer_process(cmd);
#if PLATFORM_NUM_LEDS > 0
		led_on(LED_COMMAND_PENDING);
#endif
	}
	else
		printf("\nGPS_POS itow=%u fixed=%u ttff=%u lon=%d lat=%d\n", event->pos.itow, event->pos.is_fixed, event->pos.ttff, event->pos.longitude, event->pos.latitude);

}
#endif

void bootstrap()
{
    printf("\nDevice booted\n");

#if PLATFORM_NUM_LEDS > 0
    led_off(LED_COMMAND_PENDING);
    led_off(LED_COMMAND_RECEIVED);
    led_off(LED_GPS_TRACKING);
    led_off(LED_GPS_FIXED);
#endif

    d7ap_fs_init();
    d7ap_init();

    alp_layer_init(&alp_init_args, false);
    d7ap_fs_write_dll_conf_active_access_class(0x11); // use scanning AC

#ifdef PLATFORM_USE_NEO_M8N_GPS
    gps_init(gps_event_handler);
    gps_enable();
    gps_disable();
#endif

#if PLATFORM_NUM_LEDS > 0
    sched_register_task(&led_blink_off);
#endif
}
