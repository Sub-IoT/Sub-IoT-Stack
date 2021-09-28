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

/*! \file platform_usb.c
 *
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include "usb_descriptors.h"


#include "cdc.h"
#include "em_cmu.h"
#include "em_usbd.h"
#include "em_usb.h"
#include <string.h>
#include <debug.h>


static const USBD_Callbacks_TypeDef callbacks =
{
  .usbReset        = NULL,
  .usbStateChange  = CDC_StateChangeEvent,
  .setupCmd        = CDC_SetupCmd,
  .isSelfPowered   = NULL,
  .sofInt          = NULL
};

const USBD_Init_TypeDef usbInitStruct =
{
  .deviceDescriptor    = &USBDESC_deviceDesc,
  .configDescriptor    = USBDESC_configDesc,
  .stringDescriptors   = USBDESC_strings,
  .numberOfStrings     = sizeof(USBDESC_strings)/sizeof(void*),
  .callbacks           = &callbacks,
  .bufferingMultiplier = USBDESC_bufferingMultiplier,
  .reserved            = 0
};


void __usb_init_cdc()
{
	//BSP_Init(BSP_INIT_DEFAULT);   /* Initialize DK board register access */

	//CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

	/* Initialize the communication class device. */
	CDC_Init();

	/* Initialize and start USB device stack. */
	USBD_Init(&usbInitStruct);

	/*
	* When using a debugger it is practical to uncomment the following three
	* lines to force host to re-enumerate the device.
	*/
	USBD_Disconnect();
	USBTIMER_DelayMs(1000);
	USBD_Connect();

}
