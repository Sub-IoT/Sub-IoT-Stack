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
 *	\addtogroup AES
 * 	\ingroup HAL
 * @{
 * \brief Hardware interface to the on-chip hardware encryption/decryption
 * accelerator featured by the MCU of the platform.
 *
 * This header files specifies a number of cryptographic functions rendered by
 * the hardware cryptography engine.
 */
#ifndef __HW_AES_H_
#define __HW_AES_H_
#include "types.h"
#include "errors.h"
#include "link_c.h"

/*! \brief AES Electronic Codebook (ECB) cipher mode encryption/decryption, 128 bit key.
 *
 * \param out	Buffer to place encrypted/decrypted data. Must be at least @p len long.
 * \param in	Buffer holding data to encrypt/decrypt. Must be at least @p len long
 * \param len	Number of bytes to encrypt/decrypt. Must be a multiple of 16.
 * \param key	128 bit encryption key.
 * \param encrypt	Set to true to encrypt, false to decrypt.
 */
__LINK_C void hw_aes_ecb128(uint8_t *out, const uint8_t *in, unsigned int len, const uint8_t *key, bool encrypt);

/*! \brief AES Cipher-block chaining (CBC) cipher mode encryption/decryption, 128 bit key.
 *
 * \param out	Buffer to place encrypted/decrypted data. Must be at least @p len long.
 * \param in	Buffer holding data to encrypt/decrypt. Must be at least @p len long
 * \param len	Number of bytes to encrypt/decrypt. Must be a multiple of 16.
 * \param key	128 bit encryption key.
 * \param iv	128 bit initialization vector to use.
 * \param encrypt	Set to true to encrypt, false to decrypt.
 */
__LINK_C void hw_aes_cbc128(uint8_t *out, const uint8_t *in, unsigned int len, const uint8_t *key, const uint8_t * iv, bool encrypt);


/*! \brief AES Counter (CTR) cipher mode encryption/decryption, 128 bit key.
 *
 * \param out	Buffer to place encrypted/decrypted data. Must be at least @p len long.
 * \param in	Buffer holding data to encrypt/decrypt. Must be at least @p len long
 * \param len	Number of bytes to encrypt/decrypt. Must be a multiple of 16.
 * \param key	128 bit encryption key.
 * \param ctr	128 bit initial counter block.
 */
__LINK_C void hw_aes_ctr128(uint8_t *out, const uint8_t *in, unsigned int len, const uint8_t *key, uint8_t * ctr);

#endif //__HW_AES_H_


/** @}*/
