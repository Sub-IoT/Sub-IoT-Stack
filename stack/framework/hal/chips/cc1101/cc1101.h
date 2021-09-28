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

/*! \file
 *
 *
 */

#ifndef CC1101_H
#define CC1101_H

#define FIFO_SIZE   64

#define AVAILABLE_BYTES_IN_TX_FIFO  20
#define BYTES_IN_RX_FIFO            20

/* \brief Callback called by cc1101_interface_{spi/cc430} when end_of_packet interrupt occurs.
 * Note: this is called from an interrupt context so should contain minimal processing.
 *
 */
typedef void(*end_of_packet_isr_t)(void);
typedef void(*fifo_thr_isr_t)(void);

#endif // CC1101_H
