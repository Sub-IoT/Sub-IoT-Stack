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

#include "hwsystem.h"

#include <msp430.h>
 #include <tlv.h>
 
#include <assert.h>

void hw_enter_lowpower_mode(uint8_t mode)
{
    switch(mode)
    {
        case 0:
        {
            __bis_SR_register(LPM0_bits | GIE);
            break;
        }
        case 1:
        {
            __bis_SR_register(LPM1_bits | GIE);
            break;
        }
        case 2:
        {
            __bis_SR_register(LPM2_bits | GIE);
            break;
        }
        case 3:
        {
            __bis_SR_register(LPM3_bits | GIE);
            break;
        }
        case 4:
        {
            __bis_SR_register(LPM4_bits | GIE);
            break;
        }
        default:
        {
            assert(0);
        }
    }
}

uint64_t hw_get_unique_id()
{
    struct s_TLV_Die_Record * pDIEREC;
    unsigned char bDieRecord_bytes;

    TLV_getInfo(TLV_TAG_DIERECORD,
        0,
        &bDieRecord_bytes,
        (unsigned int **)&pDIEREC
        );

    unsigned char tagId[8];
    unsigned char* pointer = (unsigned char*) &(pDIEREC->wafer_id);
    tagId[0] = pointer[3];
    tagId[1] = pointer[2];
    tagId[2] = pointer[1];
    tagId[3] = pointer[0];
    pointer = (unsigned char*) &(pDIEREC->die_x_position);
    tagId[4] = pointer[1];
    tagId[5] = pointer[0];

    pointer = (unsigned char*) &(pDIEREC->die_y_position);
    tagId[6] = pointer[1];
    tagId[7] = pointer[0];
}

void hw_busy_wait(int16_t microseconds)
{
    // TODO measure accuracy
    while (microseconds > 0)
    {
        __delay_cycles(4);
        microseconds--;
    }
}

    