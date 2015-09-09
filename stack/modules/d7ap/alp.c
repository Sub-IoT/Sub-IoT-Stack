/*! \file alp.c
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
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
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include "assert.h"
#include "ng.h"

#include "alp.h"


static alp_unhandled_action_callback NGDEF(_unhandled_action_cb);
#define unhandled_action_cb NG(_unhandled_action_cb)

static alp_operation_t get_operation(uint8_t* alp_command)
{
    alp_control_t alp_ctrl;
    alp_ctrl.raw = (*alp_command);
    return alp_ctrl.operation;
}

void alp_init(alp_unhandled_action_callback cb)
{
    unhandled_action_cb = cb;
}

bool alp_process_received_command_d7asp(d7asp_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size)
{
    // TODO split into actions

    assert(get_operation(alp_command) == ALP_OP_RETURN_FILE_DATA); // TODO other operations not supported yet

    if(unhandled_action_cb)
        unhandled_action_cb(d7asp_result, alp_command, alp_command_size);
}
