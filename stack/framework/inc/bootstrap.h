/* \file
 *
 * Bootstrapping functions for the framework
 *
 */
#ifndef __BOOTSTRAP_H_
#define __BOOTSTRAP_H_

#include "scheduler.h"


/*! \brief callback from the HAL to initialise the framework
 *
 * This function is not a 'true' task in the sense that it is not registered with the 
 * scheduler. As a result it will only be executed once. 
 *
 * It should be noted that this function is respobnsible for initialising the scheduler. 
 * This is NOT done by the HAL, since the scheduler is part of the framework.
 *
 */
void __framework_bootstrap();

/*! \brief The task scheduled to 'bootstrap' the application.
 *
 * The 'bootstrap' must be provided by the application and is called once the
 * HAL, Framework and included modules have been initialised.
 *
 * This task is only scheduled once, it is the responsibility of the application
 * to reschedule this task or schedule additional tasks if needed
 *
 */
task_t bootstrap();

#endif