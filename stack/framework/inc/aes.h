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

/*!
 * \file aes.h
 * \addtogroup aes
 * \ingroup framework
 * @{
 * \brief The AES module provides encryption/decryption functions.
 * \author philippe.nunes@cortus.com
 */

#ifndef _AES_H_
#define _AES_H_

#include <types.h>

#define AES_BLOCK_SIZE 16

// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES128 encryption in CBC-mode of operation and handles 0-padding.
// ECB enables the basic ECB 16-byte block algorithm. Both can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
  #define CBC 0 // We don't need this mode in the latest DASH7 specification
#endif

#ifndef ECB
  #define ECB 1
#endif

#ifndef CTR
  #define CTR 1
#endif

void AES128_init(const uint8_t *key);

#if defined(ECB) && ECB

// The two functions AES128_ECB_xxcrypt() do most of the work, and they expect inputs of 128 bit length.
void AES128_ECB_encrypt(uint8_t *input, uint8_t *output);
void AES128_ECB_decrypt(uint8_t *input, uint8_t *output);

#endif // #if defined(ECB) && ECB


#if defined(CBC) && CBC

void AES128_CBC_encrypt_buffer(uint8_t *output, uint8_t *input, uint32_t length, const uint8_t *iv);
void AES128_CBC_decrypt_buffer(uint8_t *output, uint8_t *input, uint32_t length, const uint8_t *iv);

#endif // #if defined(CBC) && CBC

#if defined(CTR) && CTR
void AES128_CTR_encrypt(uint8_t *output, uint8_t *input, uint32_t length, uint8_t* ctr_blk);
// Decryption is exactly the same operation as encryption

#endif // #if defined(CTR) && CTR

/*! \brief AES CBC-MAC (Cipher Block Chaining MAC).
 *
 * \param auth		Buffer to place the MAC. Must be at least @p auth_len long.
 * \param payload	Buffer to place the text to authenticate.
 * \param length	Number of bytes to encrypt. Must be a multiple of 16.
 * \param iv		Initialization vector to be used as the first block by CBC-MAC
 * \param add		Buffer to place the Additional authenticated data.
 * \param add_len	Length of the additional authentication data
 * \param auth_len	MIC length of 0, 4, 8 or 16 bytes are allowed
 */
error_t AES128_CBC_MAC( uint8_t *auth, uint8_t *payload, uint8_t length, const uint8_t *iv,
                        const uint8_t *add, uint8_t add_len, uint8_t auth_len );


/*! \brief AES Counter with CBC-MAC (CCM), 128 bit key.
 *
 * \param payload	Buffer to place the plain text. The encrypted data is overwritten on this buffer. Must be at least @p len long.
 * \param length	Number of bytes to encrypt. Must be a multiple of 16.
 * \param iv		Initialization vector to be used as the first block by CBC-MAC
 * \param add		Buffer to place the Additional authenticated data.
 * \param add_len	Length of the additional authentication data
 * \param ctr_blk	128 bit initial counter block to be used for the CTR encryption.
 * \param auth_len	MIC length of 0, 4, 8 or 16 bytes are allowed
 */
error_t AES128_CCM_encrypt( uint8_t *payload, uint8_t length, const uint8_t *iv,
                            const uint8_t *add, uint8_t add_len, uint8_t *ctr_blk,
                            uint8_t auth_len );

/*! \brief AES Counter with CBC-MAC (CCM), 128 bit key.
 *
 * \param payload	Buffer to place the encrypted text. The decrypted data is overwritten on this buffer. Must be at least @p len long.
 * \param length	Number of bytes to decrypt. Must be a multiple of 16.
 * \param iv		Initialization vector to be used as the first block by CBC-MAC
 * \param add		Buffer to place the Additional authenticated data.
 * \param add_len	Length of the additional authentication data
 * \param ctr_blk	128 bit initial counter block to be used for the CTR encryption.
 * \param auth      Authentication Tag to check
 * \param auth_len	MIC length of 0, 4, 8 or 16 bytes are allowed
 */
error_t AES128_CCM_decrypt( uint8_t *payload, uint8_t length, const uint8_t *iv,
                            const uint8_t *add, uint8_t add_len, uint8_t *ctr_blk,
                            const uint8_t *auth, uint8_t auth_len );

#endif //_AES_H_

/** @}*/
