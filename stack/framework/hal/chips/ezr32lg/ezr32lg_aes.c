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

/*! \file ezr32lg_aes.c
 *
 *  \author philippe.nunes@cortus.com
 *
 */


#include "hwaes.h"
#include "ezr32lg_chip.h"
#include <em_aes.h>

#define AES_BLOCKSIZE 16

typedef void (*AES_CtrFuncPtr_TypeDef)(uint8_t *ctr);

/* Increment AES counter */
void IncrementAesCounterBlock(uint8_t * ctr_blk)
{
    int i;

    /* in network byte order so start at end and work back */
    for (i = AES_BLOCKSIZE - 1; i >= 0; i--) {
        if (++ctr_blk[i])  /* we're done unless we overflow */
            return;
    }
}

__LINK_C void hw_aes_ecb128(uint8_t *out, const uint8_t *in, unsigned int len, const uint8_t *key, bool encrypt)
{
	AES_ECB128(out, in, len, key, encrypt);
}

__LINK_C void hw_aes_cbc128(uint8_t *out, const uint8_t *in, unsigned int len, const uint8_t *key, const uint8_t * iv, bool encrypt)
{
	AES_CBC128(out, in, len, key, iv, encrypt);
}

__LINK_C void hw_aes_ctr128(uint8_t *out, const uint8_t *in, unsigned int len, const uint8_t *key, uint8_t * ctr)
{
	AES_CTR128(out, in, len, key, ctr, &IncrementAesCounterBlock);
}
