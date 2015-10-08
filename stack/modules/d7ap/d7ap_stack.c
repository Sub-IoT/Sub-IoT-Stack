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

#include "debug.h"

void d7ap_stack_init(fs_init_args_t* fs_init_args, d7asp_init_args_t* d7asp_init_args)
{
    assert(fs_init_args != NULL);
    assert(fs_init_args->access_profiles_count > 0); // there should be at least one access profile defined

    fs_init(fs_init_args);
    d7asp_init(d7asp_init_args);
    d7atp_init();
    packet_queue_init();
    dll_init();
}
