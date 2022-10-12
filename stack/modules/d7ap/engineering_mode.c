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

#include "engineering_mode.h"
#include "MODULE_D7AP_defs.h"
#include "framework_defs.h"

#include "d7ap.h"
#include "log.h"
#include "d7ap_fs.h"
#include "phy.h"
#include "packet.h"
#include "crc.h"
#include "packet_queue.h"

#include "modem_interface.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_EM_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_EM, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif

// Define the maximum length of the user data according the size occupied already by the parameters length, counter, id and crc
#define PACKET_METADATA_SIZE (2*sizeof(uint16_t) /* word for crc + counter */  + 1 /*byte length*/)
#define PACKET_SIZE 10
#if(PACKET_SIZE <= 5)
# error
#endif

#define FILL_DATA_SIZE PACKET_SIZE - PACKET_METADATA_SIZE
#define PER_PACKET_DELAY 20

typedef struct packet packet_t;

static uint8_t timeout_em = 0;
static phy_tx_config_t tx_cfg;
static phy_rx_config_t rx_cfg;
static bool stop = false;

static uint16_t per_missed_packets_counter = 0;
static uint16_t per_received_packets_counter = 0;
static uint16_t per_packet_counter = 0; 
static uint16_t per_start_index = 65535; //Impossible value to show this is not yet set
static uint16_t per_packet_limit = 0;
static uint8_t per_data[PACKET_SIZE] = { [0 ... PACKET_SIZE-1]  = 0 };
static uint8_t per_fill_data[FILL_DATA_SIZE + 1];
typedef struct {
  union {
    uint8_t per_packet_buffer[sizeof(hw_radio_packet_t) + 255];
    hw_radio_packet_t hw_radio_packet;
  };
} per_packet_t;
static per_packet_t per_packet;
static engineering_mode_t active_mode = EM_OFF;

static void start_mode();
static void stop_mode();

static void cont_tx_done_callback(packet_t* packet) { 
  phy_switch_to_sleep_mode();
}

static void packet_transmitted_callback(packet_t* packet) {
  DPRINT("packet %i transmitted", per_packet_counter);
  if(per_packet_counter >= per_packet_limit && per_packet_limit != 25500) { //timeout of 255 = unlimited
    DPRINT("PER test done");
    return;
  }

  timer_post_task_delay(&start_mode, PER_PACKET_DELAY);
}

static void packet_received_em(packet_t* packet) {
  uint16_t crc = __builtin_bswap16(crc_calculate(packet->hw_radio_packet.data, packet->hw_radio_packet.length - 2));
  if(memcmp(&crc, packet->hw_radio_packet.data + packet->hw_radio_packet.length - 2, 2) != 0)
  {
      per_missed_packets_counter++;
      DPRINT("##fault##");
  }
  else
  {
      uint16_t msg_counter = 0;
      uint16_t data_len = packet->hw_radio_packet.length - sizeof(msg_counter) - 2;

      uint8_t rx_data[FILL_DATA_SIZE+1];
      if(data_len > sizeof(rx_data))
      {
        per_missed_packets_counter++;
        DPRINT("##fault##");
      }
      else
      {
        memcpy(&msg_counter, packet->hw_radio_packet.data + 1, sizeof(msg_counter));
        memcpy(rx_data, packet->hw_radio_packet.data + 1 + sizeof(msg_counter), data_len);
        
        if((per_start_index == 65535) || (msg_counter == 1)) {
            per_start_index = msg_counter - 1;

            // just start, assume received all previous counters to reset PER to 0%
            per_received_packets_counter = 0;
            per_packet_counter = 0;
            per_missed_packets_counter = 0;
        }

        uint16_t expected_counter = per_packet_counter + 1 + per_start_index;
        if(msg_counter == expected_counter)
        {
            per_received_packets_counter++;
            per_packet_counter++;
        }
        else if(msg_counter > expected_counter)
        {
            per_missed_packets_counter += msg_counter - expected_counter;
            per_packet_counter = msg_counter - per_start_index;
        }
        else
        {
            sched_post_task(&start_mode);
        }

        double per = 0;
        assert((msg_counter - per_start_index) != 0); 
        if(msg_counter > 0)
            per = 100.0 - ((double)per_received_packets_counter / (double)(msg_counter - per_start_index)) * 100.0;
        
        if(msg_counter % 5 == 0) {
          char to_uart_uint[40];
          sprintf(to_uart_uint, "PER %i%%. Counter %i, rssi %idBm      ", (int)per, msg_counter, packet->hw_radio_packet.rx_meta.rssi);
  #ifdef FRAMEWORK_MODEM_INTERFACE_ENABLED
          modem_interface_transfer_bytes((uint8_t*)to_uart_uint, 40, 0x04); //SERIAL_MESSAGE_TYPE_LOGGING
  #endif
          DPRINT("PER = %i%%\n counter <%i>, rssi <%idBm>, length <%i>, timestamp <%lu>\n", (int)per, msg_counter, packet->hw_radio_packet.rx_meta.rssi, packet->hw_radio_packet.length + 1, packet->hw_radio_packet.rx_meta.timestamp);
        }
    }
  }
  packet_queue_free_packet(packet);
}

static void start_mode() {
  switch (active_mode)
  {
    case EM_OFF:
      hw_reset();
      break;
    case EM_CONTINUOUS_TX:
      phy_continuous_tx(&tx_cfg, timeout_em, &cont_tx_done_callback);
      break;
    case EM_TRANSIENT_TX:
      if(!stop) {
        timer_post_task_delay(&start_mode, 1200);

        phy_continuous_tx(&tx_cfg, 1, &cont_tx_done_callback);
      }
      break;
    case EM_PER_RX:
      phy_start_rx(&(rx_cfg.channel_id), rx_cfg.syncword_class, &packet_received_em);
      break;
    case EM_PER_TX:
      DPRINT("transmitting packet");

      per_packet_counter++;
      per_data[0] = sizeof(per_packet_counter) + FILL_DATA_SIZE + sizeof(uint16_t); /* CRC is an uint16_t */
      memcpy(per_data + 1, &per_packet_counter, sizeof(per_packet_counter));
      /* the CRC calculation shall include all the bytes of the frame including the byte for the length*/
      memcpy(per_data + 1 + sizeof(per_packet_counter), per_fill_data, FILL_DATA_SIZE);
      uint16_t crc = __builtin_bswap16(crc_calculate(per_data, per_data[0] + 1 - 2));
      memcpy(per_data + 1 + sizeof(per_packet_counter) + FILL_DATA_SIZE, &crc, 2);
      memcpy(&per_packet.hw_radio_packet.data, per_data, sizeof(per_data));
      per_packet.hw_radio_packet.length = per_data[0] + 1;
      error_t e = phy_send_packet(&per_packet.hw_radio_packet, &tx_cfg, &packet_transmitted_callback);
      break;
    case EM_CONTINUOUS_STANDBY:
      phy_switch_to_standby_mode();
      break;
  }
}

static void stop_mode() {
  switch (active_mode)
  {
    case EM_TRANSIENT_TX:
      stop = true;
      break;
    case EM_CONTINUOUS_STANDBY:
      phy_switch_to_sleep_mode();
      break;
    default:
      log_print_error_string("we can't 'stop' mode %i", active_mode);
      break;
  }
}

static void em_file_change_callback(uint8_t file_id) {
    uint8_t data[D7A_FILE_ENGINEERING_MODE_SIZE];
    uint32_t length = D7A_FILE_ENGINEERING_MODE_SIZE;
    d7ap_fs_read_file(D7A_FILE_ENGINEERING_MODE_FILE_ID, 0, data, &length, ROOT_AUTH);

    d7ap_fs_engineering_mode_t* em_command = (d7ap_fs_engineering_mode_t*)data;
    em_command->channel_id.center_freq_index = __builtin_bswap16(em_command->channel_id.center_freq_index);

    DPRINT("em_file_change_callback");
    DPRINT_DATA(data, D7A_FILE_ENGINEERING_MODE_SIZE);

    rx_cfg.syncword_class = PHY_SYNCWORD_CLASS1;
    tx_cfg.syncword_class = PHY_SYNCWORD_CLASS1;

    timeout_em = em_command->timeout;

    active_mode = em_command->mode;

    switch (em_command->mode)
    {
      case EM_OFF:
        DPRINT("EM_MODEM_OFF");
        timer_post_task_delay(&start_mode, 500);
        break;
      case EM_CONTINUOUS_TX:
        DPRINT("EM_MODE_CONTINUOUS_TX\n");
        memcpy( &(tx_cfg.channel_id), &(em_command->channel_id), sizeof(channel_id_t));
        tx_cfg.eirp = em_command->eirp;

        DPRINT("Tx: %d seconds, coding: %X \nclass: %X, freq band: %X \nchannel id: %d, syncword class: %X \neirp: %d, flags: %X\n",
        timeout_em, tx_cfg.channel_id.channel_header.ch_coding, tx_cfg.channel_id.channel_header.ch_class,
        tx_cfg.channel_id.channel_header.ch_freq_band, tx_cfg.channel_id.center_freq_index,
        tx_cfg.syncword_class, tx_cfg.eirp, em_command->flags);

        /* start the radio */
        //give it time to answer through uart
        timer_post_task_delay(&start_mode, 500);
        break;
      case EM_TRANSIENT_TX:
        DPRINT("EM_MODE_TRANSIENT_TX\n");
        memcpy( &(tx_cfg.channel_id), &(em_command->channel_id), sizeof(channel_id_t));
        tx_cfg.eirp = em_command->eirp;

        stop = false;
        if(timeout_em != 0) {
          timer_post_task_delay(&stop_mode, timeout_em * TIMER_TICKS_PER_SEC + 500);
        }

        //give it time to answer through uart
        timer_post_task_delay(&start_mode, 500);
        break;
      case EM_PER_RX:
        DPRINT("EM_MODE_PER_RX\n");
        per_packet_counter = 0;
        per_missed_packets_counter = 0;
        per_received_packets_counter = 0;
        per_start_index = 65535;
        rx_cfg.channel_id = em_command->channel_id;
        sched_post_task(&start_mode);
        break;
      case EM_PER_TX:
        DPRINT("EM_MODE_PER_TX\n");
        per_packet_counter = 0;
        tx_cfg.channel_id = em_command->channel_id;
        tx_cfg.eirp = em_command->eirp;
        per_packet_limit = timeout_em * 100;
        hw_radio_set_idle();
        timer_post_task_delay(&start_mode, 500);
        break;
      case EM_CONTINUOUS_STANDBY:
        DPRINT("EM_MODEM_CONTINUOUS_STANDBY");
        if(timeout_em != 0) {
            timer_post_task_delay(&stop_mode, timeout_em * TIMER_TICKS_PER_SEC + 500);
        }
        timer_post_task_delay(&start_mode, 500);
        break;
    }
}

error_t engineering_mode_init()
{
  // always init EM file to 0 to avoid bricking the device
  uint8_t init_data[D7A_FILE_ENGINEERING_MODE_SIZE] = {0};
  d7ap_fs_write_file(D7A_FILE_ENGINEERING_MODE_FILE_ID, 0, init_data, D7A_FILE_ENGINEERING_MODE_SIZE, ROOT_AUTH);

  d7ap_fs_register_file_modified_callback(D7A_FILE_ENGINEERING_MODE_FILE_ID, &em_file_change_callback);

  sched_register_task(&start_mode);
  sched_register_task(&stop_mode);

  return SUCCESS;
}

error_t engineering_mode_stop()
{
  d7ap_fs_unregister_file_modified_callback(D7A_FILE_ENGINEERING_MODE_FILE_ID);

  return SUCCESS;
}
