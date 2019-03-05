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


// This examples pushes sensor data to gateway(s) by manually constructing an ALP command with a file read result action
// (unsolicited message). The D7 session is configured to request ACKs. All received ACKs are printed.
// Temperature data is used as a sensor value, when a HTS221 is available, otherwise value 0 is used.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hwleds.h"
#include "hwsystem.h"
#include "hwlcd.h"

#include "scheduler.h"
#include "timer.h"
#include "debug.h"
#include "fs.h"
#include "log.h"

#include "d7ap.h"
#include "alp_layer.h"
#include "dae.h"
#include "stdbool.h"

#include "emmacro.h"
#include "emtype.h"
#include "TimersL1.h"
#include "ComL1.h"
#include "ModbusRtuSlaveL7.h"
#include "ModbusRtuSlaveIntrL6.h"

static alp_init_args_t alp_init_args;
void some_other_task()
{
    ModbusRtuSlaveIntrL6_TimerHandler();
    timer_post_task_delay(&some_other_task, 100);
}
void some_task()
{
        ModbusRtuSlaveIntrL7_WorkerThread();
        sched_post_task(&some_task);
}

void bootstrap()
{
    log_print_string("Device booted\n");
    //TimersL1_Init();

	ComL1_OpenPort(ComL1_GetPortHandle(M_COM_MODBUS), 9600u);
	ModbusRtuSlaveIntrL7_Reset();
	ModbusRtuSlaveIntrL7_Open(1u);
    sched_register_task(&some_other_task);
    sched_register_task(&some_task);
    timer_post_task_delay(&some_other_task, 100);
    sched_post_task(&some_task);
}