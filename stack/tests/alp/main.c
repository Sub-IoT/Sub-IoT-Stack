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
 *
 *  \author glenn.ergeerts@aloxy.io
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

#include "alp.h"

// TODO define here now, since we are not using APP_BUILD() macro for tests
const char _APP_NAME[] = "alp_test";
const char _GIT_SHA1[] = "";

void test_alp_parse_length_operand()
{
    fifo_t fifo;
    uint8_t data[10] = { 0 };
    fifo_init(&fifo, data, sizeof(data));

    fifo_put_byte(&fifo, 0x01);
    uint32_t length;
    assert(alp_parse_length_operand(&fifo, &length));
    assert(length == 1);

    fifo_clear(&fifo);
    fifo_put_byte(&fifo, 0x40);
    fifo_put_byte(&fifo, 0x41);
    assert(alp_parse_length_operand(&fifo, &length));
    assert(length == 65);

    fifo_clear(&fifo);
    fifo_put_byte(&fifo, 0x80);
    fifo_put_byte(&fifo, 0x40);
    fifo_put_byte(&fifo, 0x01);
    assert(alp_parse_length_operand(&fifo, &length));
    assert(length == 0x4001);

    fifo_clear(&fifo);
    fifo_put_byte(&fifo, 0xC0);
    fifo_put_byte(&fifo, 0x41);
    fifo_put_byte(&fifo, 0x10);
    fifo_put_byte(&fifo, 0x00);
    assert(alp_parse_length_operand(&fifo, &length));
    assert(length == 4263936);
}

void bootstrap()
{
    printf("Unit-tests for ALP\n");

    printf("Testing alp_parse_length_operand ... ");
    test_alp_parse_length_operand();
    printf("Success!\n");

    printf("Unit-tests for ALP completed\n");

    exit(0);
}
