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

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include "platform_defs.h"
#include "ezr32lg_mcu.h"
#include "blockdevice_ram.h"
#include "fs.h"


#ifndef PLATFORM_EZR32LG_USB01
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_EZR32LG_USB01 to be defined
#endif


#include <ezr32lg_chip.h>

//#define HW_USE_HFXO
#define HW_USE_LFXO

/********************
 * LED DEFINITIONS *
 *******************/

#define LED_GREEN	0
#define LED_RED		1
#define LED_ORANGE	2
#define LED0	PIN(3, 6)
#define	LED1	PIN(3, 5)
#define LED2	PIN(3, 4)

/********************
 *  USB SUPPORT      *
 ********************/

#define USB_DEVICE


// UART0 location #1: PE0 and PE1
//#define UART_PIN_TX         E0           // PE0
//#define UART_PIN_RX         E1          // PE1

/*******************
 *   VCOM          *
 ******************/



/********************
 * SPI RF DEFINITIONS *
 *******************/

#define si4455_GDO0_PIN A15
#define si4455_GDO1_PIN E14
#define si4455_SDN_PIN  E8

/*************************
 * DEBUG PIN DEFINITIONS *
 ************************/

#define DEBUG_PIN_NUM 0
#define DEBUG0	PIN(4, 0)
#define DEBUG1	PIN(4, 1)

/**************************
 * USERBUTTON DEFINITIONS *
 *************************/

#define BUTTON0				PIN(5, 5)


extern uint8_t d7ap_files_data[FRAMEWORK_FS_PERMANENT_STORAGE_SIZE];
extern uint8_t d7ap_volatile_files_data[FRAMEWORK_FS_VOLATILE_STORAGE_SIZE];

static blockdevice_ram_t permanent_bd = (blockdevice_ram_t){
 .base.driver = &blockdevice_driver_ram,
 .base.size = FRAMEWORK_FS_PERMANENT_STORAGE_SIZE,
 .buffer = d7ap_files_data
};

static blockdevice_ram_t volatile_bd = (blockdevice_ram_t){
 .base.driver = &blockdevice_driver_ram,
 .base.size = FRAMEWORK_FS_VOLATILE_STORAGE_SIZE,
 .buffer = d7ap_volatile_files_data
};

/** Platform BD drivers*/
#define PLATFORM_PERMANENT_BD (blockdevice_t*)&permanent_bd
#define PLATFORM_VOLATILE_BD (blockdevice_t*)&volatile_bd


#endif
