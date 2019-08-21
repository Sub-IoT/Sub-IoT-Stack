/*! \file main.c
 *
 *  \copyright (C) Copyright 2019 Aloxy
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
 *
 *  \author glenn.ergeerts@aloxy.io
 */

#include <stdio.h>
#include <string.h>
#include "alp_layer.h"
#include "debug.h"
#include "fifo.h"
#include "log.h"


#define DPRINT(...) printf(__VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)

// used by FS, normally defined only in apps by cmake, so define here for unit test
const char _GIT_SHA1[] = "-";
const char _APP_NAME[] = "unittest";

static void alp_response_callback(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* itf_cfg) // TODO (uint16_t trans_id, uint8_t* payload, uint8_t payload_length, alp_interface_status_t* itf_status)
{
    DPRINT("Received ALP resp for %i:\n", trans_id);
    DPRINT_DATA(payload, payload_length);
}

void bootstrap(void* arg)
{
    DPRINT("Unit-tests for ALP\n");

    alp_init_args_t args;
    args.transmit_cb = &alp_response_callback;

    alp_layer_init(&args, false);
    d7ap_fs_init();

    uint8_t cmd[100];
    fifo_t fifo;
    fifo_init(&fifo, cmd, sizeof (cmd));
    alp_append_read_file_data_action(&fifo, 0, 0, 8, false, false);
    uint8_t resp[100];
    uint8_t resp_len = 0;
    alp_interface_config_t itf_cfg = (alp_interface_config_t){
        .alp_itf_id = ALP_ITF_ID_HOST
    };

    alp_layer_process_command(cmd, (uint8_t)fifo_get_size(&fifo), &itf_cfg, NULL);
    DPRINT_DATA(resp, resp_len);

//    dae_access_profile_t ap;
//    d7ap_fs_read_access_class(0, &ap);

}
