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

/*! \file engineering_mode.h
 * \addtogroup ENGINEERING_MODE_H
 * \ingroup D7AP
 * @{
 * \brief D7AP engineering mode
 * \author   maarten.weyn@aloxy.io
 */


#ifndef ENGINEERING_MODE_H
#define ENGINEERING_MODE_H

#include "types.h"
#include "phy.h"

typedef struct __attribute__((__packed__))
{
  uint8_t mode;
  uint8_t flags;
  uint8_t timeout;
  channel_id_t channel_id;
  int8_t eirp;
} d7ap_fs_engineering_mode_t;

error_t engineering_mode_init(); 
error_t engineering_mode_stop();

#endif //ENGINEERING_MODE_H

/** @}*/