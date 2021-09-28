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

/*! \file platf_main.c
 *
 *  \author contact@christophe.vg
 *
 */

#include "ezr32lg_mcu.h"

#include "hwgpio.h"
#include "hwleds.h"

#include "scheduler.h"
#include "bootstrap.h"

#include "platform.h"

#include "led.h"

void __platform_init() {
	__ezr32lg_mcu_init();
  __gpio_init();
  __led_init();
}

void __platform_post_framework_init() {
  led_init();
}

int main(){
  // initialise the platform itself
	__platform_init();

  // do not initialise the scheduler, this is done by __framework_bootstrap()
  __framework_bootstrap();

  // initialise platform functionality that depends on the framework
  __platform_post_framework_init();

  scheduler_run();
  return 0;
}
