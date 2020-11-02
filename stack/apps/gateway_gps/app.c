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

/*
 * \author	Liam Wickins <liamw9534@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hwuart.h"
#include "hwleds.h"
#include "hwsystem.h"
#include "hwlcd.h"

#include "scheduler.h"
#include "timer.h"
#include "log.h"
#include "debug.h"
#include "d7ap_fs.h"
#include "fifo.h"
#include "version.h"
#include "compress.h"

#include "platform_defs.h"

#include "d7ap.h"
#include "alp_layer.h"
#include "dae.h"
#include "modem_interface.h"
#include "platform.h"

#ifdef PLATFORM_USE_NEO_M8N_GPS
#include "gps.h"
#endif


#define GPS_POS_FILE_ID		 0x40
#define GPS_ENABLE_FILE_ID       0x41
#define GPS_DISABLE_FILE_ID      0x42


// LEDs used to indicate device status
#define LED_COMMAND_PENDING      0
#define LED_COMMAND_RECEIVED     0
#define LED_GPS_TRACKING	 2
#define LED_GPS_FIXED            3


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
        .access_class = 0x11,
        .id = { }
    }
  }
};

static bool gps_remote_enable = false;


#if PLATFORM_NUM_BUTTONS > 0

#include "button.h"

#endif

#if PLATFORM_NUM_LEDS > 0
  #include "hwleds.h"

  void led_blink_off()
  {
    led_off(LED_COMMAND_RECEIVED);
  }

  void led_blink()
  {
    led_on(LED_COMMAND_RECEIVED);

    timer_post_task_delay(&led_blink_off, TIMER_TICKS_PER_SEC * 0.2);
  }
#endif


void send_task()
{
	uint8_t file_id;
	alp_command_t *cmd;
	cmd = alp_layer_command_alloc(false, false);
	alp_append_forward_action(cmd, (alp_interface_config_t*)&itf_config, sizeof(itf_config));
	file_id = gps_remote_enable ? GPS_DISABLE_FILE_ID : GPS_ENABLE_FILE_ID;
	alp_append_write_file_data_action(cmd, file_id, 0, 0, NULL, false, false);
	alp_layer_process(cmd);
}

static error_t host_interface_send(uint8_t* payload, uint8_t payload_length,
	  __attribute__((__unused__)) uint8_t expected_response_length,
	  __attribute__((__unused__)) uint16_t* trans_id,
	  __attribute__((__unused__)) alp_interface_config_t* session_config)
{
#if PLATFORM_NUM_LEDS > 0
	led_blink();
#endif
	printf("\nD7ALP message opcode=%02x file_id=%02x offset=%u len=%u\n", payload[0], payload[1], payload[2], payload[3]);

#ifdef PLATFORM_USE_NEO_M8N_GPS
	if (payload[0] == ALP_OP_RETURN_FILE_DATA)
	{
		if (payload[1] == GPS_POS_FILE_ID && payload[3] >= sizeof(gps_event_t))
		{
			gps_event_t *event = (gps_event_t *)&payload[4];
			printf("\nGPS_POS: itow=%u fixed=%u ttff=%u lon=%d lat=%d\n", event->pos.itow, event->pos.is_fixed, event->pos.ttff, event->pos.longitude, event->pos.latitude);
#if PLATFORM_NUM_LEDS > 0
	    	led_on(LED_GPS_TRACKING);
	    	if (event->pos.is_fixed)
	    		led_on(LED_GPS_FIXED);
	    	else
	    		led_off(LED_GPS_FIXED);
#endif
		}
	}
#endif
	return 0;
}

void host_interface_register() {
	static alp_interface_t alp_host_interface = {
        .itf_id = ALP_ITF_ID_HOST,
        .itf_cfg_len = 0,
        .itf_status_len = 0,
        .send_command = host_interface_send,
        .init = NULL,
        .deinit = NULL,
        .unique = false
    };

    alp_layer_register_interface(&alp_host_interface);
}

#if PLATFORM_NUM_BUTTONS > 0

static void button_callback(button_id_t button_id)
{
    printf("\nButton pressed - request %u\n", !gps_remote_enable);
    send_task();
}
#endif

void on_alp_command_completed_cb(uint8_t tag_id, bool success)
{
    if (success)
    {
      printf("\nCommand (%i) completed successfully\n", tag_id);
      gps_remote_enable = !gps_remote_enable;

      if (gps_remote_enable)
      {
#if PLATFORM_NUM_LEDS > 0
    	  led_on(LED_GPS_TRACKING);
#endif
      }
      else
      {
#if PLATFORM_NUM_LEDS > 0
    	  led_off(LED_GPS_TRACKING);
    	  led_off(LED_GPS_FIXED);
#endif
      }
    }
    else
    {
    	printf("\nCommand failed, no ack received\n");
    }
#if PLATFORM_NUM_LEDS > 0
    led_off(LED_COMMAND_PENDING);
#endif

#if PLATFORM_NUM_BUTTONS == 0
    timer_post_task_delay(&send_task, TIMER_TICKS_PER_SEC * 1);
#endif

}

static alp_init_args_t alp_init_args = {
	.alp_command_completed_cb = on_alp_command_completed_cb,
};


void bootstrap()
{
  printf("\nDevice booted\n");

#if PLATFORM_NUM_LEDS > 0
  led_off(LED_COMMAND_PENDING);
  led_off(LED_COMMAND_RECEIVED);
  led_off(LED_GPS_TRACKING);
  led_off(LED_GPS_FIXED);
#endif

#if PLATFORM_NUM_BUTTONS > 0
  ubutton_register_callback(0, button_callback);
#endif

  d7ap_fs_init();
  d7ap_init();

  d7ap_fs_write_dll_conf_active_access_class(0x01); // set to first AC, which is continuous FG scan

  host_interface_register();

  alp_layer_init(&alp_init_args, false);

#ifdef HAS_LCD
  lcd_write_string("GW %s", _GIT_SHA1);
#endif

#if PLATFORM_NUM_LEDS > 0
  sched_register_task(&led_blink_off);
#endif

#if PLATFORM_NUM_BUTTONS == 0
  sched_register_task(&send_task);
  timer_post_task_delay(&send_task, TIMER_TICKS_PER_SEC * 0);
#endif
}

