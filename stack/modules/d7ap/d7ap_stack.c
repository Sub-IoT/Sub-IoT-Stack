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

#include "d7ap_stack.h"
#include "shell.h"
#include "debug.h"
#include "framework_defs.h"


void d7ap_stack_init(fs_init_args_t* fs_init_args, alp_init_args_t* alp_init_args, bool enable_shell, alp_cmd_handler_appl_itf_callback alp_cmd_handler_appl_itf_cb)
{
    assert(fs_init_args != NULL);
    assert(fs_init_args->access_profiles_count > 0); // there should be at least one access profile defined

    fs_init_args->fs_d7aactp_cb = &alp_layer_process_d7aactp;
    fs_init(fs_init_args);
    d7asp_init();
    d7atp_init();
    d7anp_init();
    packet_queue_init();
    dll_init();

    alp_layer_init(alp_init_args, enable_shell);

    uint8_t read_firmware_version_alp_command[] = { 0x01, D7A_FILE_FIRMWARE_VERSION_FILE_ID, 0, D7A_FILE_FIRMWARE_VERSION_SIZE };

    if(!enable_shell)
    {
#ifdef MODULE_D7AP_BROADCAST_VERSION_ON_BOOT_ENABLED
      // notify booted by broadcasting and retrying 3 times (for diagnostics ie to detect reboots)
      // TODO: default access class
      d7asp_master_session_config_t broadcast_fifo_config = {
          .qos = {
            .qos_resp_mode                = SESSION_RESP_MODE_ANY,
            .qos_retry_mode               = SESSION_RETRY_MODE_NO,
            .qos_record                   = false,
            .qos_stop_on_error            = false
          },
          .dormant_timeout = 0,
          .addressee = {
            .ctrl = {
                .nls_method               = AES_NONE,
                .id_type                  = ID_TYPE_NOID,
            },
            .access_class                 = 0x01,
            .id = 0
          }
      };

      uint8_t alp_response[ALP_PAYLOAD_MAX_SIZE] = { 0 };
      uint8_t alp_response_length = 0;
      alp_process_command_result_on_d7asp(&broadcast_fifo_config, read_firmware_version_alp_command, sizeof(read_firmware_version_alp_command), ALP_CMD_ORIGIN_APP); // TODO origin stack?
#endif
    }
}

void d7ap_stack_stop()
{
    d7asp_stop();
    d7atp_stop();
    d7anp_stop();
    dll_stop();
    hw_radio_stop();
}
