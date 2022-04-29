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
#include "fifo.h"
#include "assert.h"
#include "errors.h"
#include "stdio.h"

#define BUFFER_SIZE 10

void test_peek()
{
    fifo_t test_fifo;
    uint8_t element;  
    uint8_t buffer[BUFFER_SIZE] = {0,1,2,3,4,5,6,7,8,9};
    uint8_t tmp[BUFFER_SIZE] = {0,};
    

    fifo_init_filled(&test_fifo, buffer, BUFFER_SIZE, BUFFER_SIZE);
    assert(fifo_is_full(&test_fifo) == true);

    for(int i=0; i < BUFFER_SIZE; i++)
    {
        element = 10;
        assert(fifo_peek(&test_fifo, &element, i, 1) == SUCCESS);
        // printf("%d: real: %d, expected: %d \n", i, element, buffer_filled[i]);
        assert(element == buffer[i]);
        assert(fifo_get_size(&test_fifo) == BUFFER_SIZE);
    }

    assert(fifo_peek(&test_fifo, tmp, 0, 11) == ESIZE);
    assert(fifo_peek(&test_fifo, tmp, 1, 10) == ESIZE);

    // test peek multiple elements
    assert(fifo_peek(&test_fifo, tmp, 2, 2) == SUCCESS); // 2, 3
    assert(tmp[0] == 2);
    assert(tmp[1] == 3);
    assert(tmp[2] == 0);
}

void test_pop()
{
    fifo_t test_fifo;
    uint8_t element;  
    uint8_t buffer[BUFFER_SIZE] = {0,1,2,3,4,5,6,7,8,9};
    uint8_t expected[BUFFER_SIZE] = {0,1,2,3,4,5,6,7,8,9};
    uint8_t buff[BUFFER_SIZE] = {0};

    // single element pop
    fifo_init_filled(&test_fifo, buffer, BUFFER_SIZE, BUFFER_SIZE);
    assert(fifo_is_full(&test_fifo) == true);

    for(int i=0; i < BUFFER_SIZE; i++)
    {
        element = 10;
        assert(fifo_pop(&test_fifo, &element, 1) == SUCCESS);
        assert(element == buffer[i]);
        assert(fifo_is_full(&test_fifo) == false);
        // printf("size: %d, full: %d, head: %d, tail: %d \n", fifo_get_size(&test_fifo), fifo_is_full(&test_fifo), (test_fifo.head_idx), ( test_fifo.tail_idx));
        assert(fifo_get_size(&test_fifo) == 10 - i - 1);
    }
    assert(fifo_pop(&test_fifo, &element, 1) == ESIZE);

    //pop multiple elements
    fifo_init_filled(&test_fifo, buffer, BUFFER_SIZE, BUFFER_SIZE);
    assert(fifo_is_full(&test_fifo) == true);

    assert(fifo_pop(&test_fifo, buff, 9) == SUCCESS);
    assert(fifo_get_size(&test_fifo) == 1);
    for(int i = 0; i < 9; i++){
        assert(expected[i] == buff[i]);
    }
    
    assert(fifo_pop(&test_fifo, buff, 2) == ESIZE);
}

void test_put()
{
    fifo_t test_fifo;
    uint8_t element;  
    uint8_t buffer[BUFFER_SIZE] = {0,};
    uint8_t expected[BUFFER_SIZE] = {0,1,2,3,4,5,6,7,8,9};

    // test single element access
    fifo_init(&test_fifo, buffer, BUFFER_SIZE);
    assert(fifo_is_full(&test_fifo) == false);
    assert(fifo_get_size(&test_fifo) == 0);

    for(int i=0; i < BUFFER_SIZE; i++)
    {
        assert(fifo_put(&test_fifo, &expected[i], 1) == SUCCESS);
        assert(fifo_get_size(&test_fifo) == i + 1);
        assert(buffer[i] == expected[i]);
    }
    assert(fifo_put(&test_fifo, &expected[0], 1) == ESIZE);

    // test put multiple elements
    fifo_init(&test_fifo, buffer, BUFFER_SIZE);
    assert(fifo_is_full(&test_fifo) == false);
    assert(fifo_get_size(&test_fifo) == 0);

    assert(fifo_put(&test_fifo, expected, 5) == SUCCESS);
    assert(fifo_get_size(&test_fifo) == 5);
    for(int i = 0; i < 5; i++){
        assert(buffer[i] == expected[i]);
    }
    
    assert(fifo_put(&test_fifo, expected, 6) == ESIZE);

}

void test_circular()
{
    fifo_t test_fifo;
    uint8_t element;  
    uint8_t buffer[BUFFER_SIZE] = {0,1,2,3,4,5,6,7,8,9};
    
    fifo_init_filled(&test_fifo, buffer, BUFFER_SIZE, BUFFER_SIZE);
    assert(fifo_is_full(&test_fifo) == true);

    uint8_t expected[BUFFER_SIZE] = {4,5,6,7,8,9,0,1,2,3}; 
    for(int i=0; i < 4; i++)
    {
        element = 10;
        assert(fifo_pop(&test_fifo, &element, 1) == SUCCESS);
        assert(fifo_is_full(&test_fifo) == false);
        assert(fifo_put(&test_fifo, &element, 1) == SUCCESS);
        assert(fifo_is_full(&test_fifo) == true);
    }
    for(int i=0; i < BUFFER_SIZE; i++)
    {
        assert(fifo_pop(&test_fifo, &element, 1) == SUCCESS);
        assert(fifo_get_size(&test_fifo) == 10 - i - 1);
        assert(element == expected[i]);
    }
    assert(fifo_pop(&test_fifo, &expected[0], 1) == ESIZE);

    assert(fifo_put(&test_fifo, expected, 6) == SUCCESS);
    assert(fifo_get_size(&test_fifo) == 6);
    uint8_t expected_new[6] = {4,5,6,7,8,9}; 
    uint8_t buff[6] = {0,};
    assert(fifo_pop(&test_fifo, buff, 6) == SUCCESS);
    assert(fifo_get_size(&test_fifo) == 0);
    for(int i=0; i < 6; i++)
    {
        assert(buff[i] == expected_new[i]);
    }
   
}

void test_subview()
{
    fifo_t test_fifo;
    fifo_t subview;
    uint8_t element;
    uint8_t buff[BUFFER_SIZE]   = {0,};
    uint8_t buffer[BUFFER_SIZE] = {0,1,2,3,4,5,6,7,8,9};
    
    fifo_init_filled(&test_fifo, buffer, BUFFER_SIZE, BUFFER_SIZE);
    assert(fifo_init_subview(&subview, &test_fifo, 0, 11) == ESIZE);
    assert(fifo_init_subview(&subview, &test_fifo, 3, 8) == ESIZE);
    

    uint8_t expected0[BUFFER_SIZE] = {0,1,2,3,4,5,6,7,8,9};
    assert(fifo_init_subview(&subview, &test_fifo, 0, 10) == SUCCESS);
    assert(fifo_pop(&subview, buff, 10) == SUCCESS);
    for(int i=0; i < 10; i++){
        assert(buff[i] == expected0[i]);
    }
    assert(fifo_pop(&subview, buff, 1) == ESIZE);

    uint8_t expected1[BUFFER_SIZE] = {3,4,5,6,7};
    assert(fifo_init_subview(&subview, &test_fifo, 3, 5) == SUCCESS);
    assert(fifo_pop(&subview, buff, 5) == SUCCESS);
    for(int i=0; i < 5; i++){
        assert(buff[i] == expected1[i]);
    }

    fifo_pop(&test_fifo, buff, 9); //test_fifo = {9}
    fifo_put(&test_fifo, expected1, 5); //test_fifo = {9,3,4,5,6,7}
    uint8_t expected2[BUFFER_SIZE] = {5,6,7};
    assert(fifo_init_subview(&subview, &test_fifo, 3, 3) == SUCCESS);
    assert(fifo_pop(&subview, buff, 3) == SUCCESS);
    for(int i=0; i < 3; i++){
        assert(buff[i] == expected2[i]);
    }

}

void test_remove_last_byte()
{
    fifo_t test_fifo;
    fifo_t subview;
    uint8_t element;
    uint8_t buff[BUFFER_SIZE]   = {0,};
    uint8_t buffer[BUFFER_SIZE] = {0,1,2,3,4,5,6,7,8,9};

    
    fifo_init_filled(&test_fifo, buffer, BUFFER_SIZE, BUFFER_SIZE);

    uint8_t expected0[BUFFER_SIZE] = {0,1,2,3,4,5,6,7,8};
    fifo_init_subview(&subview, &test_fifo, 0, 10);
    assert(fifo_remove_last_byte(&subview) == SUCCESS);
    // printf("size: %d, full: %d, head: %d, tail: %d \n", fifo_get_size(&test_fifo), fifo_is_full(&test_fifo), (test_fifo.head_idx), ( test_fifo.tail_idx));
    assert(fifo_get_size(&subview) == 9);
    for(int i=0; i < 9; i++)
    {
        assert(fifo_pop(&subview, &element, 1) == SUCCESS);
        assert(expected0[i] == element);
    }
    assert(fifo_remove_last_byte(&subview) == ESIZE);

    uint8_t expected1[BUFFER_SIZE] = {6,7,8,9,2,3,4};
    assert(fifo_pop(&test_fifo, buff, 6) == SUCCESS);  //{6,7,8,9}
    assert(fifo_put(&test_fifo, &buff[2], 4) == SUCCESS); //{6,7,8,9,2,3,4,5}
    assert(fifo_remove_last_byte(&test_fifo) == SUCCESS);
    assert(fifo_get_size(&test_fifo) == 7);
    for(int i=0; i < 7; i++)
    {
        assert(fifo_pop(&test_fifo, &element, 1) == SUCCESS);
        assert(expected1[i] == element);
    }

    

}

void test_pop_empty()
{
    fifo_t test_fifo;
    uint8_t buffer[BUFFER_SIZE] = {0,1,2,3,4,5,6,7,8,9};
    uint8_t expected[BUFFER_SIZE] = {0,1,2,3,4,5,6,7,8,9};
    uint8_t buff[BUFFER_SIZE] = {0};

    // single element pop
    fifo_init(&test_fifo, buffer, BUFFER_SIZE);
    assert(fifo_get_size(&test_fifo) == 0);
    assert(fifo_pop(&test_fifo, buff, 0) == SUCCESS);
    assert(fifo_get_size(&test_fifo) == 0);
}

int main(int argc, char *argv[])
{
    printf("Testing fifo_peek ... ");
    test_peek();
    printf("Success!\n");

    printf("Testing fifo_pop ... ");
    test_pop();
    printf("Success!\n");

    printf("Testing fifo_put ... ");
    test_put();
    printf("Success!\n");

    printf("Testing circularity ... ");
    test_circular();
    printf("Success!\n");

    printf("Testing subview ... ");
    test_subview();
    printf("Success!\n");

    printf("Testing fifo_remove_last_byte ... ");
    test_remove_last_byte();
    printf("Success!\n");

    printf("Testing fifo_pop with current size 0 ... ");
    test_pop_empty();
    printf("Success!\n");

    printf("All FIFO tests passed!\n");

}
