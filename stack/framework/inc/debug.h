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

/*! \file debug.h
 * \addtogroup debug
 * \ingroup framework
 * @{
 * \brief Debugging helpers like DEBUG_ASSERT() which functions like libc's assert() but
 * can be optimized to use less code size by defining FRAMEWORK_DEBUG_ASSERT_MINIMAL.
 *
 */
#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "framework_defs.h"
#include "assert.h" // start from assert.h which defines __ASSERT_FUNCT and then redefine assert
#include "error_event_file.h"
#include "hwsystem.h"
#include <stdio.h>
#undef assert

#ifdef __ASSERT_FUNCTION  // some systems define __ASSERT_FUNCTION ..
#   define __ASSERT_FUNC __ASSERT_FUNCTION
void __assert_func(const char *, int, const char *, const char *);

#endif

#ifndef __ASSERT_FUNC
#   define __ASSERT_FUNC ((char *) 0)
void __assert_func(const char *, int, const char *, const char *);
#endif

#ifdef NDEBUG           /* required by ANSI standard */
# define assert(__e) (__e)
#elif defined FRAMEWORK_DEBUG_ASSERT_REBOOT
#if defined(FRAMEWORK_USE_CALLSTACK) && defined(FRAMEWORK_USE_ERROR_EVENT_FILE)
# define assert(__e) \
  do { \
    if(!(__e)) { \
      error_event_create_assert_event(); \
      hw_reset(); \
    } \
  } while(0)
#else
# define assert(__e) \
  do { \
    if(!(__e)) { \
      hw_reset(); \
    } \
  } while(0)
#endif
#elif defined FRAMEWORK_DEBUG_ASSERT_MINIMAL
# define assert(__e) ((__e) ? (void)0 : __assert_func (NULL, 0, NULL, NULL))
#else
# define assert(__e) ((__e) ? (void)0 : __assert_func (__FILE__, __LINE__, \
                               __ASSERT_FUNC, #__e))
#endif

#endif // __DEBUG_H_

/** @}*/
