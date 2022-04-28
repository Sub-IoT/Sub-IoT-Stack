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

// Copyright Aloxy.io
// @brief: Contains stack trace functionality
#include "callstack.h"
#include "unwind.h"

uintptr_t callstack_stack_pointer;

extern uintptr_t _estack;
extern uintptr_t __stack_start;

typedef struct
{
    uint8_t cstack_len;
    uint8_t max_cstack_len;
    uint8_t frames_to_skip;
    uintptr_t last_ip;
    intptr_t saved_lr;
    uintptr_t* callstack;
}strace_context_t;

// This struct definition mimics the internal structures of libgcc in
// arm-none-eabi binary. It's not portable and might break in the future.
struct core_regs
{
    uint32_t r[16];
};

// This struct definition mimics the internal structures of libgcc in
// arm-none-eabi binary. It's not portable and might break in the future.
typedef struct
{
    uint32_t demand_save_flags;
    struct core_regs core;
} phase2_vrs_t;

extern _Unwind_Reason_Code __gnu_Unwind_Backtrace(_Unwind_Trace_Fn trace, void *trace_argument, phase2_vrs_t *entry_vrs);

// Takes registers from the stack which are pushed by the CPU when the interrupt occurred and
// fills in the structure necessary for the LIBGCC unwinder.
static void fill_phase2_vrs(intptr_t context, phase2_vrs_t* phase2_vrs, intptr_t* saved_lr)
{
    volatile intptr_t* fault_args = (intptr_t*)context;
    phase2_vrs->demand_save_flags = 0;
    phase2_vrs->core.r[0] = fault_args[0];
    phase2_vrs->core.r[1] = fault_args[1];
    phase2_vrs->core.r[2] = fault_args[2];
    phase2_vrs->core.r[3] = fault_args[3];
    phase2_vrs->core.r[12] = fault_args[4];
    // We add +2 here because first thing libgcc does with the lr value is
    // subtract two, presuming that lr points to after a branch
    // instruction. However, exception entry's saved PC can point to the first
    // instruction of a function and we don't want to have the backtrace end up
    // showing the previous function.
    phase2_vrs->core.r[14] = fault_args[6] + 2;
    phase2_vrs->core.r[15] = fault_args[6];
    *saved_lr = fault_args[5];
    phase2_vrs->core.r[13] = (unsigned)(fault_args + 8); // stack pointer
}

// Callback from the unwind backtrace function.
_Unwind_Reason_Code trace_func(struct _Unwind_Context *context, void *arg)
{
    strace_context_t* strace_context = (strace_context_t*)arg;
    uintptr_t ip;
    ip = _Unwind_GetIP(context);
    if (strace_context->cstack_len != 0 && strace_context->last_ip == ip)
    {
        if (strace_context->cstack_len == 1 && (unsigned int) strace_context->saved_lr != _Unwind_GetGR(context, 14))
        {
            _Unwind_SetGR(context, 14, strace_context->saved_lr);
            return _URC_NO_REASON;
        }
        return _URC_END_OF_STACK;
    }
    if (strace_context->cstack_len >= strace_context->max_cstack_len)
    {
        return _URC_END_OF_STACK;
    }
    strace_context->last_ip = ip;
    // This gives the start of the function back
    // ip = (void *)_Unwind_GetRegionStart(context);
    if(strace_context->frames_to_skip > 0)
    {
        strace_context->frames_to_skip--;
    }
    else
    {
        strace_context->callstack[strace_context->cstack_len++] = ip;
    }
    return _URC_NO_REASON;
}

uint8_t callstack_from_isr(uintptr_t* cstack_buffer, uint8_t cstack_buffer_size)
{
    // Check if captured stackpointer is pointing to a location on the stack
    if((((uint32_t)&_estack) < callstack_stack_pointer) || (callstack_stack_pointer < ((uint32_t)&__stack_start)))
    {
        return 0;
    }
    // Check if current stackpointer is still lower than captured stackpointer
    uintptr_t current_stack_pointer;
    __asm volatile("MOV %0, sp\n"
        : "=r" (current_stack_pointer)
    );
    
    if(current_stack_pointer > callstack_stack_pointer)
    {
        // stacktrace_stack_pointer is not pointing to a part that is on the active stack
        return 0;
    }
    strace_context_t strace_context;
    strace_context.cstack_len = 0;
    strace_context.max_cstack_len = cstack_buffer_size;
    strace_context.last_ip = (uintptr_t)NULL;
    strace_context.callstack = cstack_buffer;
    strace_context.frames_to_skip = 0;
    // Make variable static so that it will not be placed on the stack
    static phase2_vrs_t phase2_vrs;
    fill_phase2_vrs(callstack_stack_pointer, &phase2_vrs,&strace_context.saved_lr);

    // phase2_vrs_t first_context = phase2_vrs;
    // __gnu_Unwind_Backtrace(&trace_func, 0, &first_context);
    __gnu_Unwind_Backtrace(&trace_func, &strace_context, &phase2_vrs);

    // This is a workaround for the case when the function in which we had the
    // exception trigger does not have a stack saved LR. In this case the
    // backtrace will fail after the first step. We manually append the second
    // step to have at least some idea of what's going on.
    if (strace_context.cstack_len == 1)
    {
        phase2_vrs.core.r[14] = strace_context.saved_lr;
        phase2_vrs.core.r[15] = strace_context.saved_lr;
        __gnu_Unwind_Backtrace(&trace_func, &strace_context, &phase2_vrs);
    }
    return strace_context.cstack_len;
}

uint8_t callstack(uintptr_t* cstack_buffer, uint8_t cstack_buffer_size, uint8_t skip_functions)
{
    strace_context_t strace_context;
    strace_context.cstack_len = 0;
    strace_context.max_cstack_len = cstack_buffer_size;
    strace_context.last_ip = (uintptr_t)NULL;
    strace_context.callstack = cstack_buffer;
    strace_context.frames_to_skip = 1 + skip_functions;
    _Unwind_Backtrace(&trace_func, &strace_context);
    return strace_context.cstack_len;
}