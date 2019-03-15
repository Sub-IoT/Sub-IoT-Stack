/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2019 Aloxy 
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

/*! \file d7ap_em.h
 * \addtogroup D7AP_EM
 * \ingroup D7AP
 * @{
 * \brief D7AP enginieering mode
 * \author   maarten.weyn@aloxy.io
 */


#ifndef D7AP_EM_H
#define D7AP_EM_H

// first byte is the mode
#define EM_MODE_OFF           0
#define EM_MODE_CONTINOUS_TX  1
#define EM_MODE_CONTINOUS_RX  2

// command
// for EM_MODE_CONTINOUS_TX
// 0 timout in secs (0 means always on - dangerous...)
// 2-6 hw_tx_cfg_t
// 7-7 modulation_t

// for EM_MODE_CONTINOUS_RX
// 0 timout in secs (0 means always on - dangerous...)
// 2-5 hw_rx_cfg_t

typedef enum {
  MODULATION_CW,
  MODULATION_GFSK,
} modulation_t;

#include "types.h"
#include "MODULE_D7AP_defs.h"

error_t em_init(); 


#endif //D7AP_EM_H

/** @}*/