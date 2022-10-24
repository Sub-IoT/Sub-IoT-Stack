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

/*
 * \author	maarten.weyn@uantwerpen.be
 */

#include <stdio.h>
#include <stdlib.h>

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


static alp_interface_config_d7ap_t itf_config = (alp_interface_config_d7ap_t){
  .itf_id = ALP_ITF_ID_D7ASP,
  .d7ap_session_config = {
    .qos = {
        .qos_resp_mode = SESSION_RESP_MODE_ALL,
        .qos_retry_mode = SESSION_RETRY_MODE_NO
    },
    .dormant_timeout = 0x3F,
    .addressee = {
        .ctrl = {
            .nls_method = AES_NONE,
            .id_type = ID_TYPE_UID,
        },
        .access_class = 0x21,
        .id = { 0x35, 0x32, 0x30, 0x39, 0x00, 0x3E, 0x00, 0x27 }
    }
  }
};

static void add_dormant_session()
{
  alp_command_t* command = alp_layer_command_alloc(true, false);

  // forward to the D7 interface
  alp_append_forward_action(command, (alp_interface_config_t*)&itf_config, sizeof(itf_config));

  // add the return file data action
  uint8_t data[] = {0, 1};
  alp_append_write_file_data_action(command, 0x48, 0, 2, data, true, false);

  // and finally execute this
  alp_layer_process(command);

  log_print_string("\ndormant session should be added\n");
}

#if PLATFORM_NUM_LEDS > 0
  #include "hwleds.h"

  void led_blink_off()
  {
    led_off(0);
  }

  void led_blink(alp_command_t* command, alp_interface_status_t* origin_itf_status)
  {
    log_print_string("gotten message from interface 0x%X", origin_itf_status ? origin_itf_status->itf_id : 0xFF);
    led_on(0);

    log_print_string("command %i completed %i with error %i ", command->tag_id, command->is_response_completed, command->is_response_error);

    timer_post_task_delay(&led_blink_off, TIMER_TICKS_PER_SEC * 0.2);

    timer_post_task_delay(&add_dormant_session, TIMER_TICKS_PER_SEC);
  }
  static alp_init_args_t alp_init_args = { .alp_command_result_cb = &led_blink };
#else
  static alp_init_args_t alp_init_args;
#endif


void bootstrap()
{
  alp_layer_init(&alp_init_args, true);

  d7ap_fs_write_dll_conf_active_access_class(0x01); // set to first AC, which is continuous FG scan

  sched_register_task(&add_dormant_session);

#ifdef HAS_LCD
  lcd_write_string("GW %s", _GIT_SHA1);
#endif

#if PLATFORM_NUM_LEDS > 0
  sched_register_task(&led_blink_off);
#endif
}

