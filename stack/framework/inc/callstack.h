/*
 * Copyright (c) 2015-2022 University of Antwerp, Aloxy NV.
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
 
#ifndef __CALLSTACK_H_
#define __CALLSTACK_H_

#include "types.h"

extern uintptr_t callstack_stack_pointer;

/*! \brief Fills the given buffer with a call stack starting from an earlier captured place on the stack
 *
 * This function will write a call stack to cstack_buffer. The given call stack represents the call stack before a certain interrupt occurred.
 * The stack pointer on the moment of the interrupt should be stored in callstack_stack_pointer just before the interrupt service routine is executed.
 * The entries in the call stack contain the address of the next instruction in each function.
 *
 * \param cstack_buffer		    A pointer to a buffer that can contain multiple addresses.
 * \param cstack_buffer_size    The maximum number of addresses that the buffer can contain. 
 * \returns uint8_t	            The amount of addresses written to cstack_buffer.
 *
 */
uint8_t callstack_from_isr(uintptr_t* cstack_buffer, uint8_t cstack_buffer_size);

/*! \brief Fills the given buffer with the current call stack
 *
 * This function will write a call stack to cstack_buffer.
 * The entries in the call stack contain the address of the next instruction in each function.
 *
 * \param cstack_buffer		    A pointer to a buffer that can contain multiple addresses.
 * \param cstack_buffer_size    The maximum number of addresses that the buffer can contain. 
 * \param skip_functions        The amount of functions to skip before starting with the call stack
 * \returns uint8_t	            The amount of addresses written to cstack_buffer.
 *
 */
uint8_t callstack(uintptr_t* cstack_buffer, uint8_t cstack_buffer_size, uint8_t skip_functions);

#endif // __CALLSTACK_H_