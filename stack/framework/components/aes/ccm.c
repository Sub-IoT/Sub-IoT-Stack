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

/*
 * This is an implementation of the Counter with CBC-MAC (CCM) with AES.
 */


/*****************************************************************************/
/* Includes:                                                                 */
/*****************************************************************************/
#include <stdint.h>
#include <string.h> // CBC mode, for memset
#include "stdbool.h"
#include "aes.h"
#include "types.h"
#include "errors.h"
#include "log.h"
#include "framework_defs.h"


#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_AES_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_PHY, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif

/* Implementation according RFC 3610: Counter with CBC-MAC (CCM)
 * Counter with CBC-MAC (CCM) is a generic authenticated encryption
 * block cipher mode
 */

static void xor_aes_block(uint8_t *dst, const uint8_t *src)
{
    uint8_t i;

    for (i = 0; i < AES_BLOCK_SIZE; ++i)
    {
        dst[i] ^= src[i];
    }
}

/*
 * Authentication
 *
 * To secure CBC-MAC for variable length messages, the first block (B_0)
 * contains the length of the message. 
 * 
 */

error_t AES128_CBC_MAC( uint8_t *auth, uint8_t *payload, uint8_t length, const uint8_t *iv,
                        const uint8_t *add, uint8_t add_len, uint8_t auth_len )
{
    uint8_t blk[AES_BLOCK_SIZE];
    uint8_t i;
    uint8_t remainders;
    uint8_t tag[AES_BLOCK_SIZE];

    /* sanity checks */
    if (auth_len != 4 && auth_len != 8 && auth_len != 16)
        return EINVAL;

    if (add_len > (2 * AES_BLOCK_SIZE - 1))
        return EINVAL;

    /* For DASH7, the payload length shall be less than 250 - authentication tag len */
    if (length > (250 - auth_len))
        return EINVAL;

    /* The CBC-MAC is computed by:
     *
     * X_1 := E( K, B_0 )
     * X_i+1 := E( K, X_i XOR B_i )  for i=1, ..., n
     * T := first-M-bytes( X_n+1 )
     */

    /* X_1 = E(K, B_0) */
    DPRINT("Blk0");
    DPRINT_DATA((uint8_t *)iv, AES_BLOCK_SIZE);
    AES128_ECB_encrypt((uint8_t *)iv, tag);
    DPRINT("X_1 = AES(B_0)");
    DPRINT_DATA(tag, AES_BLOCK_SIZE);

    // if add_len > 0, add more blocks of authentication data
    if (add_len > 0)
    {
        uint8_t use_len = add_len < AES_BLOCK_SIZE - 1 ? add_len : AES_BLOCK_SIZE - 1;
        remainders = add_len - use_len;

        memset(blk, 0, AES_BLOCK_SIZE);
        // For DASH7, the additional data length shall be encoded in a field of 1 octet.
        blk[0] = add_len;

        memcpy( blk + 1, add, use_len );

        DPRINT("Blk1");
        DPRINT_DATA(blk, AES_BLOCK_SIZE);

        xor_aes_block(blk, tag);
        DPRINT("X_1 XOR B_1");
        DPRINT_DATA(blk, AES_BLOCK_SIZE);
        /* X_2 = E(K, X_1 XOR B_1) */
        AES128_ECB_encrypt(blk, tag);
        DPRINT("X_2 = AES(X_1 XOR B_1)");
        DPRINT_DATA(tag, AES_BLOCK_SIZE);

        if (remainders)
        {
            memset(blk, 0, AES_BLOCK_SIZE);
            memcpy(blk, add + use_len, remainders);

            DPRINT("blk2");
            DPRINT_DATA(blk, AES_BLOCK_SIZE);

            DPRINT("X_2 XOR B_2");
            xor_aes_block(blk, tag);
             /* X_3 = E(K, X_2 XOR B_2) */
            AES128_ECB_encrypt(blk, tag);
            DPRINT("X_3 = AES(X_1 XOR B_1)");
            DPRINT_DATA(tag, AES_BLOCK_SIZE);
        }
    }

    remainders = length % AES_BLOCK_SIZE; /* Remaining bytes in the last non-full block */
    DPRINT("Remainders %d length %d", remainders, length);

    for (i = 0; i < length / AES_BLOCK_SIZE; i++)
    {
        DPRINT("Xi");
        DPRINT_DATA(tag, AES_BLOCK_SIZE);

        DPRINT("B_i");
        DPRINT_DATA(payload, AES_BLOCK_SIZE);

        /* X_i+1 = E(K, X_i XOR B_i) */
        xor_aes_block(tag, payload);
        DPRINT("X_i XOR B_i");
        DPRINT_DATA(tag, AES_BLOCK_SIZE);

        payload += AES_BLOCK_SIZE;

        AES128_ECB_encrypt(tag, tag);
        DPRINT("X_i+1 = E(K, X_i XOR B_i)");
        DPRINT_DATA(tag, AES_BLOCK_SIZE);
    }

    if (remainders)
    {
        /* XOR zero-padded last block */
        DPRINT("Xi");
        DPRINT_DATA(tag, AES_BLOCK_SIZE);

        DPRINT("B_i");
        DPRINT_DATA(payload, AES_BLOCK_SIZE);

        for (i = 0; i < remainders; i++)
            tag[i] ^= *payload++;
        DPRINT("X_i XOR B_i");
        DPRINT_DATA(tag, AES_BLOCK_SIZE);

        AES128_ECB_encrypt(tag, tag);
        DPRINT("X_i+1 = E(K, X_i XOR B_i)");
        DPRINT_DATA(tag, AES_BLOCK_SIZE);
    }

    memcpy(auth, tag, auth_len);

    return SUCCESS;
}

/*
 * Authenticated encryption
 *
 * Ensure that the output is sized to contain the encrypted message payload
 * + the encrypted authentication Tag.
 */
error_t AES128_CCM_encrypt( uint8_t *payload, uint8_t length, const uint8_t *iv,
                            const uint8_t *add, uint8_t add_len, uint8_t *ctr_blk,
                            uint8_t auth_len )
{
    uint8_t auth[AES_BLOCK_SIZE];
    uint8_t auth_crypted[AES_BLOCK_SIZE];
    error_t ret;

    /* sanity checks */
    if (auth_len != 4 && auth_len != 8 && auth_len != 16)
        return EINVAL;

     /* For DASH7, the payload length shall be less than 250 - Security header len - authentication tag len */
    if (length > (250 - 5 - auth_len))
        return EINVAL;

    if (add_len > (2 * AES_BLOCK_SIZE - 1))
        return EINVAL;

    /* Authentication */
    ret = AES128_CBC_MAC(auth, payload, length, iv, add, add_len, auth_len);
    if (ret != SUCCESS)
        return ret;

    DPRINT("Authentication tag:");
    DPRINT_DATA(auth, auth_len);

    /* Encryption with Counter (CTR) mode*/

    /* Encryption of the message payload, counter set to 1 */
    ctr_blk[0] = (ctr_blk[0] & 0xF0) + 1;
    DPRINT("ctr0");
    DPRINT_DATA(ctr_blk, AES_BLOCK_SIZE);

    AES128_CTR_encrypt(payload, payload, length, ctr_blk);
    DPRINT("CTR output:");
    DPRINT_DATA(payload, length);

    /* Encryption of the authentication tag , reset counter to 0*/
    ctr_blk[0] = (ctr_blk[0] & 0xF0);
    AES128_CTR_encrypt(auth_crypted, auth, auth_len, ctr_blk);
    DPRINT("Encrypted authentication tag:");
    DPRINT_DATA(auth_crypted, auth_len);
    // the 4, 8 or 16 MSB of the MAC are then appended to the payload
    memcpy(payload + length, auth_crypted, auth_len);

    return SUCCESS;
}

/*
 * Authenticated decryption
 */
error_t AES128_CCM_decrypt( uint8_t *payload, uint8_t length, const uint8_t *iv,
                            const uint8_t *add, uint8_t add_len, uint8_t *ctr_blk,
                            const uint8_t *auth, uint8_t auth_len )
{
    uint8_t T[AES_BLOCK_SIZE];
    uint8_t auth_decrypted[AES_BLOCK_SIZE];

    /* sanity checks */
    if (auth_len != 4 && auth_len != 8 && auth_len != 16)
        return EINVAL;

    /* For DASH7, the payload length shall be less than 250 - Security header len - authentication tag len */
    if (length > (250 - 5 - auth_len))
        return EINVAL;

    if (add_len > (2 * AES_BLOCK_SIZE - 1))
        return EINVAL;

    /* Decryption of the encrypted authentication Tag */
    ctr_blk[0] = (ctr_blk[0] & 0xF0);
    AES128_CTR_encrypt(auth_decrypted, (uint8_t *)auth, auth_len, ctr_blk);
    DPRINT("Decrypted authentication tag:");
    DPRINT_DATA(auth_decrypted, auth_len);

    /* Decryption of the message payload, counter set to 1 */
    ctr_blk[0] = (ctr_blk[0] & 0xF0) + 1;
    AES128_CTR_encrypt(payload, payload, length, ctr_blk);

    /* Recompute the CBC-MAC and check the authentication Tag */
    AES128_CBC_MAC(T, payload, length, iv, add, add_len, auth_len);
    DPRINT("Computed authentication tag:");
    DPRINT_DATA(T, auth_len);

    if (memcmp(T, auth_decrypted, auth_len) != 0)
    {
        DPRINT("CCM: Auth mismatch");
        return -1;
    }

    return SUCCESS;
}
