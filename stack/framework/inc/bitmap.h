
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
 * \file bitmap.h
 * \addtogroup bitmap
 * \ingroup framework
 * @{
 * \brief Bitmap helper functions
 *
 */

#ifndef BITMAP_H
#define BITMAP_H

#include "stdint.h"
#include "stdbool.h"

/*! \brief Set a bit in the bitmap
 * \param bitmap    The bitmap. Note: the user is responsible for initializing this.
 * \param pos       The bit number to set. Note: the user is responsible for checking pos is not bigger then the bitmap itself
 */
static inline void bitmap_set(uint8_t* bitmap, uint8_t pos)
{
    bitmap[pos / 8] |= (1 << (pos & 7));
}

/*! \brief Clear a bit in the bitmap
 * \param bitmap    The bitmap. Note: the user is responsible for initializing this.
 * \param pos       The bit number to clear. Note: the user is responsible for checking pos is not bigger then the bitmap itself
 */
static inline void bitmap_clear(uint8_t* bitmap, uint8_t pos)
{
    bitmap[pos / 8] &= ~(1 << (pos & 7));
}

/*! \brief Get a bit from the bitmap
 * \param bitmap    The bitmap. Note: the user is responsible for initializing this.
 * \param pos       The bit number to get. Note: the user is responsible for checking pos is not bigger then the bitmap itself
 */
static bool bitmap_get(uint8_t* bitmap, uint8_t pos)
{
    return bitmap[pos / 8] & (1 << (pos & 7))? true : false;
}

/*! \brief          Find the first occurance of 'flag' in the bitmap
 * \param bitmap    The bitmap to search
 * \param flag      Determines if we search for 1 or 0
 * \param size      The max number of bits to search
 * \return The index of the first occurance of flag, or -1 when not found
 */
static int8_t bitmap_search(uint8_t* bitmap, bool flag, uint8_t size)
{
    uint8_t i;
    for(i = 0; i < size; i++)
        if(bitmap_get(bitmap, i) == flag)
            return i;

    return -1;
}

#endif // BITMAP_H

/** @}*/
