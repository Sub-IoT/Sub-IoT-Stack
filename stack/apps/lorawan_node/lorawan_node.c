/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2017 University of Antwerp
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

#include "log.h"

#include "lorawan_stack.h"

void bootstrap()
{
    log_print_string("Device booted\n");
    lora_Init(NULL, NULL); // TODO params

    while(1)
    {
        lora_fsm();

//        DISABLE_IRQ();
//        /* if an interrupt has occurred after DISABLE_IRQ, it is kept pending
//         * and cortex will not enter low power anyway  */
//        if ( lora_getDeviceState( ) == DEVICE_STATE_SLEEP )
//        {
//        #ifndef LOW_POWER_DISABLE
//          LowPower_Handler( );
//        #endif
//        }
//        ENABLE_IRQ();
    }
}
