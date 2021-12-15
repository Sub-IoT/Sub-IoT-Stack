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

/*! \file
 * \addtogroup atomic
 * \ingroup HAL
 * @{
 * \brief the HAL API for critical sections.
 *
 * A critical section in a program's flow is executed 'atomically'
 * (as if it were a single instruction). This is usually achieved by disabling the 
 * interrupts which means that it is imperative to keep critical sections as small as 
 * possible.
 *
 * A critical section is entered by a call to 'start_atomic()' and exited through a call to
 * 'end_atomic()'
 *
 * It should be noted that critical sections can be nested and that multiple calls to start_atomic()
 * should be matched by an equal number of calls to end_atomic() before the critical section ends
 * (and the interrupts can be re-enabled). This is illustrated by the code below:
 *
 * \code{.c}
 *    void foo()
 *    {
 *	start_atomic();
 *	...
 *	end_atomic();
 *    }
 *    
 *    void bar()
 *    {
 *	start_atomic();
 *	...
 *	foo();
 *	//interrupts are still disabled despite end_atomic() being called from foo()
 *	...
 *	end_atomic();
 *	//interrupts are re-enabled
 *    }
 * \endcode
 *
 * */
#ifndef __HW_ATOMIC_H_
#define __HW_ATOMIC_H_

#include "link_c.h"

/*! \brief Start an atomic section
 *
 * See the documentation for this file for more information on the definition
 * and usage of critical sections
 *
 */
__LINK_C void start_atomic(void);

/*! \brief End an atomic section
 *
 * See the documentation for this file for more information on the definition
 * and usage of critical sections
 *
 */
__LINK_C void end_atomic(void);

/*! \brief Check if currently in atomic section
 *
 * Indicates whether we are currently in an atomic section
 */
__LINK_C _Bool in_atomic(void);

#endif //__HW_ATOMIC_H_

/** @}*/
