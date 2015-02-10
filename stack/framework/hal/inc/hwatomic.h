/*! \file
 *
 * This file contains the HAL API for critical sections.
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

/*! \brief Start an atomic section
 *
 * See the documentation for this file for more information on the definition
 * and usage of critical sections
 *
 */
void start_atomic();

/*! \brief End an atomic section
 *
 * See the documentation for this file for more information on the definition
 * and usage of critical sections
 *
 */
void end_atomic();

#endif //__HW_ATOMIC_H_
