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

/*! \file
 *
 *
 */

#ifndef CC1101_H
#define CC1101_H

/* \brief Callback called by cc1101_interface_{spi/cc430} when end_of_packet interrupt occurs.
 * Note: this is called from an interrupt context so should contain minimal processing.
 *
 */
typedef void(*end_of_packet_isr_t)(void);

#endif // CC1101_H
