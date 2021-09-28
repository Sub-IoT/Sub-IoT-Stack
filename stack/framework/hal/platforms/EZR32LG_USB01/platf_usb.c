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
#include <string.h>
#include <debug.h>
#include "hwsystem.h"
#include <stdio.h>

#include "em_usbd.h"
#include "em_usb.h"

extern void USB_IRQHandler( void );


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

	//CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);

	/* Initialize the communication class device. */
	CDC_Init();

        // store the UID in a string descriptor. This will show up as ID_SERIAL_SHORT attribute
        // in udev, and can thus be used for generating a symlink containing the device UID
        uint64_t uid = hw_get_unique_id();
        char uid_str[16];
        sprintf(uid_str, "%lx%lx", (uint32_t)(uid >> 32), (uint32_t)uid);
        for(int i = 0; i < 16; i++) {
            _usb_descr_serial.name[i] = (uid_str[i]);
        }

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
