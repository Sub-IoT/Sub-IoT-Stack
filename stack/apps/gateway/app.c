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
#include "stm32_common_eeprom.h"

static blockdevice_stm32_eeprom_t systemfiles_eeprom_blockdevice;

#if PLATFORM_NUM_LEDS > 0
  #include "hwleds.h"

  void led_blink_off()
  {
    led_off(0);
  }

  void led_blink()
  {
    led_on(0);

    timer_post_task_delay(&led_blink_off, TIMER_TICKS_PER_SEC * 0.2);
  }
#endif

static void on_unsolicited_response_received(d7ap_session_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size)
{
#if PLATFORM_NUM_LEDS > 0
  led_blink();
#endif
}

static alp_init_args_t alp_init_args;

void bootstrap()
{
   modem_interface_init(PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, (pin_id_t) 0, (pin_id_t) 0);

    systemfiles_eeprom_blockdevice = (blockdevice_stm32_eeprom_t){
      .base.driver = &blockdevice_driver_stm32_eeprom,
    };

    blockdevice_init((blockdevice_t*)&systemfiles_eeprom_blockdevice);

    d7ap_init((blockdevice_t*)&systemfiles_eeprom_blockdevice);

    d7ap_fs_write_dll_conf_active_access_class(0x01); // set to first AC, which is continuous FG scan

    alp_init_args.alp_received_unsolicited_data_cb = &on_unsolicited_response_received;
    alp_layer_init(&alp_init_args, true);

#ifdef HAS_LCD
    lcd_write_string("GW %s", _GIT_SHA1);
#endif

#if PLATFORM_NUM_LEDS > 0
    sched_register_task(&led_blink_off);
#endif
}

