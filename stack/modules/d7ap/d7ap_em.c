/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2019 Aloxy nv
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

#include "d7ap_em.h"

#include "d7ap.h"
#include "log.h"
#include "d7ap_fs.h"
#include "hwradio.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_EM_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_EM, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif

static uint8_t timeout_em = 0;


static void start_tx(){
    start_hw_radio_continuous_tx(timeout_em);
}

static void start_rx(){
    start_hw_radio_continuous_rx(timeout_em);
}

static void em_file_change_callback(uint8_t file_id){
    uint8_t data[D7A_FILE_ENGINEERING_MODE_SIZE];

    d7ap_fs_read_file(D7A_FILE_ENGINEERING_MODE_FILE_ID,0,data,D7A_FILE_ENGINEERING_MODE_SIZE);

    em_file_t* em_command = (em_file_t*) data;


    DPRINT("em_file_change_callback");
    DPRINT_DATA(data, D7A_FILE_ENGINEERING_MODE_SIZE);

    timeout_em = em_command->timeout;
  
    switch (em_command->mode)
    {
      case EM_MODE_CONTINUOUS_TX:
        DPRINT("EM_MODE_CONTINUOUS_TX");
        hw_tx_cfg_t tx_cfg;
        memcpy( &(tx_cfg.channel_id), &(em_command->channel_id), sizeof(channel_id_t));
        tx_cfg.eirp = em_command->eirp;

        DPRINT("Tx: %d seconds, coding: %X \nclass: %X, freq band: %X \nchannel id: %d, syncword class: %X \neirp: %d, flags: %X\n",
        timeout_em, tx_cfg.channel_id.channel_header.ch_coding, tx_cfg.channel_id.channel_header.ch_class,
        tx_cfg.channel_id.channel_header.ch_freq_band, tx_cfg.channel_id.center_freq_index,
        tx_cfg.syncword_class, tx_cfg.eirp, em_command->flags);
        
        /* Configure */
        hw_radio_continuous_tx(&tx_cfg, (em_command->flags & EM_FLAGS_MODULATED_MASK) == EM_FLAGS_UNMODULATED);

        /* start the radio */
        sched_register_task(&start_tx);
        //give it time to answer through uart
        timer_post_task_delay(&start_tx, 500);

        break;
      case EM_MODE_CONTINUOUS_RX:
        DPRINT("EM_MODE_CONTINUOUS_RX");
        hw_rx_cfg_t rx_cfg;
        memcpy( &(rx_cfg.channel_id), &(em_command->channel_id), sizeof(channel_id_t));

        
        /* Configure */
        hw_radio_continuous_rx(&rx_cfg);

        /* start the radio */
        sched_register_task(&start_rx);
        //give it time to answer through uart
        timer_post_task_delay(&start_rx, 500);
        break;
    }
}

error_t em_init()
{
  // always init EM file to 0 to avoid bricking the device
  uint8_t init_data[D7A_FILE_ENGINEERING_MODE_SIZE] = {0};
  d7ap_fs_write_file(D7A_FILE_ENGINEERING_MODE_FILE_ID, 0, init_data, D7A_FILE_ENGINEERING_MODE_SIZE);


  d7ap_fs_register_file_modified_callback(D7A_FILE_ENGINEERING_MODE_FILE_ID, &em_file_change_callback);
}