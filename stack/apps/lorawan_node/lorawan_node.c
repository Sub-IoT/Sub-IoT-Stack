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
#include "scheduler.h"
#include "timeServer.h"

// TODO
static uint8_t devEui[] = { 0xBE, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x81 };
static uint8_t appEui[] = { 0xBE, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x9F };
static uint8_t appKey[] = { 0x7E, 0xEF, 0x56, 0xEC, 0xDA, 0x1D, 0xD5, 0xA4, 0x70, 0x59, 0xFD, 0x35, 0x9C, 0xE6, 0x80, 0xCD };

TimerEvent_t sensor_timer;

//void lorawan_task() {
//  lorawan_stack_tick();
//  sched_post_task(&lorawan_task);
//}

void read_sensor_task() {
  static uint8_t msg_counter = 0;
  log_print_string("read sensor");
  bool success = lorawan_stack_send(&msg_counter, 1, 2, false);
  if(success) {
    log_print_string("TX ok (%i)", msg_counter);
  } else {
    log_print_string("TX failed");
  }

  msg_counter++;
  TimerStart(&sensor_timer);
}

void bootstrap()
{
    log_print_string("Device booted\n");

    lorawan_stack_init(devEui, appEui, appKey, NULL);

//    sched_register_task(&lorawan_task);
//    sched_post_task(&lorawan_task);

    TimerInit(&sensor_timer, &read_sensor_task); // TODO using lorawan stack timer API for now
    TimerSetValue(&sensor_timer, 60000);
    TimerStart(&sensor_timer);

    while(1) {
      lorawan_stack_tick();
    }
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
