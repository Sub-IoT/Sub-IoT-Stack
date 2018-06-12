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
#ifdef HAS_LCD
#include "hwlcd.h"
#endif
#include "hwsystem.h"
#include "hwradio.h"
#include "hwatomic.h"

#include "packet_queue.h"
#include "timer.h"
#include "log.h"
#include "debug.h"
#include "platform.h"
#include "shell.h"
#include "fs.h"
#include "scheduler.h"
#include "console.h"
#include "version.h"

#include "alp_cmd_handler.h"

#define PACKET_FIFO_BUFFER_SIZE (10 * sizeof(hw_radio_packet_t))

hw_radio_packet_t packet;
fifo_t packet_fifo;
uint8_t packet_fifo_buffer[PACKET_FIFO_BUFFER_SIZE];

hw_radio_packet_t* alloc_packet(uint8_t length)
{
  return &packet;
}

void free_packet(hw_radio_packet_t* packet)
{
}

void process_fifo()
{
  hw_radio_packet_t current_packet;
  start_atomic();
    while(fifo_get_size(&packet_fifo) > sizeof(hw_radio_packet_t))
    {
      fifo_pop(&packet_fifo, (uint8_t*)&current_packet, sizeof(hw_radio_packet_t));
      console_print_byte(0xC0);
      console_print_byte(1);
      console_print_bytes(current_packet.data, current_packet.length); // TODO include metadata
    }
  end_atomic();
}

void on_packet_received(hw_radio_packet_t* packet)
{
  fifo_put(&packet_fifo, (uint8_t*)packet, sizeof(hw_radio_packet_t));
  sched_post_task(&process_fifo);
}

void bootstrap()
{
  dae_access_profile_t access_classes[] = {
    {
      .control_scan_type_is_foreground = true,
      .control_csma_ca_mode = CSMA_CA_MODE_UNC,
      .control_number_of_subbands = 1,
      .subnet = 0x00,
      .scan_automation_period = 0,
      .subbands[0] = (subband_t){
        .channel_header = {
          .ch_coding = PHY_CODING_PN9,
          .ch_class = PHY_CLASS_NORMAL_RATE,
          .ch_freq_band = PHY_BAND_868
        },
        .channel_index_start = 0,
        .channel_index_end = 0,
        .eirp = 0,
        .ccao = 0
      }
    }
  };

  fs_init_args_t fs_init_args = (fs_init_args_t){
      .fs_user_files_init_cb = NULL,
      .access_profiles_count = 1,
      .access_profiles = access_classes
  };

  fs_init(&fs_init_args);

  shell_init();
  shell_register_handler((cmd_handler_registration_t){ .id = ALP_CMD_HANDLER_ID, .cmd_handler_callback = &alp_cmd_handler });

  // notify booted to serial
  uint8_t read_firmware_version_alp_command[] = { 0x01, D7A_FILE_FIRMWARE_VERSION_FILE_ID, 0, D7A_FILE_FIRMWARE_VERSION_SIZE };
  alp_process_command_console_output(read_firmware_version_alp_command, sizeof(read_firmware_version_alp_command));

  fifo_init(&packet_fifo, packet_fifo_buffer, PACKET_FIFO_BUFFER_SIZE);

  sched_register_task(&process_fifo);

  hw_radio_init(alloc_packet, free_packet);

  hw_rx_cfg_t rx_cfg = {
    .channel_id = {
      .channel_header = {
        .ch_coding = PHY_CODING_PN9,
        .ch_class = PHY_CLASS_NORMAL_RATE,
        .ch_freq_band = PHY_BAND_868
      },
      .center_freq_index = 0
    },
    .syncword_class = PHY_SYNCWORD_CLASS1
  };

  hw_radio_set_rx(&rx_cfg, &on_packet_received, NULL);

#ifdef HAS_LCD
  lcd_write_string("SNIFFER %s", _GIT_SHA1);
#endif

}

