/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2016 University of Antwerp
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

/*!
 * \file key.h
 * \ingroup framework
 * @{
 * \brief The preconfigured network security key for AES cyphering.
 * \author philippe.nunes@cortus.com
 */

#ifndef KEY_H
#define KEY_H

// preconfigured key
const uint8_t AES128_key[16] = "D7A_SECURITY_KEY";
                       //{ 0x44 0x37 0x41 0x5F 0x53 0x45 0x43 0x55 0x52 0x49 0x54 0x59 0x5f 0x4b 0x45 0x59 };
#endif // KEY_H

/** @}*/
