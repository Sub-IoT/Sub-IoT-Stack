#ifndef __HW_ATOMIC_H_
#define __HW_ATOMIC_H_

/*! \brief Start an atomic section
 *
 * The start_atomic() and end_atomic() functions define the entry- and exit point of 
 * a critical section. A critical section in a program's flow is executed 'atomically'
 * (as if it were a single instruction). This is usually achieved by disabling the 
 * interrupts which means that it is imperative to keep critical sections as small as 
 * possible.
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
 */
void start_atomic();

/*! \brief End an atomic section
 * See the documentation for start_atomic for more information
 *
 */
void end_atomic();

#endif //__HW_ATOMIC_H_
