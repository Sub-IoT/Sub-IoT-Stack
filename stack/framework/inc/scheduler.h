/*
 * scheduler.h
 *
 *  Created on: 22 Jan 2015
 *      Author: Daniel van den Akker
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdbool.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "errors.h"

typedef void (*task_t)();

void scheduler_init();
//never returns
void scheduler_run();

//not needed if we switch to the 'pure' task model
//void scheduler_init();

enum
{
	MIN_PRIORITY = 7,
	MAX_PRIORITY= 0,
};

error_t sched_register_task(task_t);

error_t sched_post_task_prio(task_t task, unsigned int priority);
static inline error_t sched_post_task(task_t task) { return sched_post_task_prio(task,MIN_PRIORITY);}
error_t sched_cancel_task(task_t task);
bool sched_is_scheduled(task_t task);


#endif /* SCHEDULER_H_ */
