/*
 * scheduler.c
 *
 *  Created on: 22 Jan 2015
 *      Author: Daniel van den Akker
 */


#include "scheduler.h"

//needs to be decoupled
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "hwint.h"
#include "ng.h"

#ifndef SCHEDULER_MAX_TASKS
    #define SCHEDULER_MAX_TASKS 64
#endif

#ifdef NODE_GLOBALS
    #warning NODE_GLOBALS is defined when using the default scheduler. Are you sure this is what you want ?
#endif

//void do_break()
//{
//	printf("Assertion Failed");
//}
//#undef assert
//#define assert(__e) ((__e) ? (void)0 : do_break())

enum
{
	NUM_PRIORITIES = MIN_PRIORITY+1,
	NUM_TASKS = SCHEDULER_MAX_TASKS,
	NO_TASK = NUM_TASKS,
	NOT_SCHEDULED = NUM_PRIORITIES,
};

typedef struct
{
	task_t task;
	uint8_t next;
	uint8_t prev;
	uint8_t priority;

} task_info_t;


typedef struct
{
	task_t task;
	uint8_t index;
} taskindex_info_t;

taskindex_info_t NGDEF(m_index)[NUM_TASKS];
task_info_t NGDEF(m_info)[NUM_TASKS];

uint8_t NGDEF(m_head)[NUM_PRIORITIES];
uint8_t NGDEF(m_tail)[NUM_PRIORITIES];
//bool NGDEF(scheduler_initialised) = false;
volatile uint8_t NGDEF(current_priority);
unsigned int NGDEF(num_registered_tasks);
#ifdef SCHEDULER_DEBUG
void check_structs_are_valid()
{
	//INT_Disable();
	hw_disable_interrupts();
	//assert(scheduler_initialised == true);
	assert(NG(num_registered_tasks) <= NUM_TASKS);
	bool visited[NUM_TASKS];
	memset(visited, false, NUM_TASKS);
	for(int i = 0; i < NG(num_registered_tasks); i++)
	{
		assert(NG(m_index)[i].task != 0x0);
		assert(NG(m_index)[i].index != NO_TASK);
		assert(NG(m_index)[i].index < NUM_TASKS);
		assert(NG(m_info)[NG(m_index)[i].index].task == NG(m_index)[i].task);
		if(!(!visited[NG(m_index)[i].index]))
		{
			printf("Error");
		}
		assert(!visited[NG(m_index)[i].index]);
		visited[NG(m_index)[i].index] = true;
	}
	for(int i = NG(num_registered_tasks); i < NUM_TASKS; i++)
	{
		assert(NG(m_index)[i].task == 0x0);
		assert(NG(m_index)[i].index == NO_TASK);
	}
	for(int i = 0; i < NUM_TASKS; i++)
		assert(visited[i] || NG(m_info)[i].task == 0x0);


	memset(visited, false, NUM_TASKS);
	for(int prio = 0; prio < NUM_PRIORITIES;prio++)
	{
		uint8_t prev_ind=NO_TASK;
		for(uint8_t cur_ind = NG(m_head)[prio]; cur_ind != NO_TASK; cur_ind = NG(m_info)[cur_ind].next)
		{
			assert(cur_ind < NUM_TASKS);
			assert(!visited[cur_ind]);
			visited[cur_ind] = true;
			assert(NG(m_info)[cur_ind].prev == prev_ind);

			if(prev_ind != NO_TASK)
				assert(NG(m_info)[prev_ind].next == cur_ind);
			else
				assert(NG(m_head)[prio] == cur_ind);

			assert(NG(m_info)[cur_ind].task != 0x0);
			assert(NG(m_info)[cur_ind].priority == prio);
			prev_ind=cur_ind;
		}
		assert(NG(m_tail)[prio] == prev_ind);
	}
	for(int i = 0; i < NUM_TASKS; i++)
	{
		if(!((visited[i]) || NG(m_info)[i].priority == NOT_SCHEDULED))
			printf("Error");
		assert((visited[i]) || NG(m_info)[i].priority == NOT_SCHEDULED);
	}

	assert(NG(current_priority) <= NUM_PRIORITIES);
	for(int i = 0; i < NG(current_priority); i++)
		assert(NG(m_head)[i] == NO_TASK);
	hw_enable_interrupts();
	//INT_Enable();
}
#else
void check_structs_are_valid(){}
#endif
void scheduler_init()
{
//	if(scheduler_initialised)
//		return;
	for(unsigned int i = 0; i < NUM_TASKS; i++)
	{
		NG(m_info)[i].next = NO_TASK;
		NG(m_info)[i].prev = NO_TASK;
		NG(m_info)[i].task = 0x0;
		NG(m_info)[i].priority = NOT_SCHEDULED;

		NG(m_index)[i].index = NO_TASK;
		NG(m_index)[i].task = 0x0;
	}
	memset(NG(m_head), NO_TASK, sizeof(NG(m_head)));
	memset(NG(m_tail), NO_TASK, sizeof(NG(m_tail)));
	NG(current_priority) = NUM_PRIORITIES;
	NG(num_registered_tasks) = 0;
//	scheduler_initialised=true;
	check_structs_are_valid();
}

static uint8_t get_task_id(task_t task)
{
	check_structs_are_valid();
	assert(NG(num_registered_tasks) <= NUM_TASKS);
	int begin = 0;
	int end = NG(num_registered_tasks)-1;
	uint32_t i = 0;

	while (begin <= end)
	{
		int pivot = begin + ((end - begin) / 2);
		if (NG(m_index)[pivot].task < task)
			begin= pivot+1;
		else if (task < NG(m_index)[pivot].task)
			end = pivot-1;
		else
		{
			return NG(m_index)[pivot].index;
		}
		i++;
		assert(i < 10000);
	}
	return NO_TASK;
}

error_t sched_register_task(task_t task)
{
	error_t retVal;
	check_structs_are_valid();
	//INT_Disable();
	hw_disable_interrupts();
	if(NG(num_registered_tasks) >= NUM_TASKS)
		retVal = ENOMEM;
	else if(get_task_id(task) != NO_TASK)
		retVal = EALREADY;
	else
	{
		for(int i = NG(num_registered_tasks); i >= 0; i--)
		{
			if (i == 0 || NG(m_index)[i-1].task < task)
			{
				NG(m_index)[i].task = task;
				NG(m_index)[i].index = NG(num_registered_tasks);
				NG(m_info)[NG(m_index)[i].index].task = task;
				break;
			}
			else
			{
				NG(m_index)[i] = NG(m_index)[i-1];
			}
		}
		NG(num_registered_tasks)++;
		retVal = SUCCESS;
	}
	//INT_Enable();
	hw_enable_interrupts();
	check_structs_are_valid();
	return retVal;
}

static inline bool is_scheduled(uint8_t id)
{
	assert(id < NUM_TASKS);
	check_structs_are_valid();
	return NG(m_info)[id].priority != NOT_SCHEDULED;
}

bool sched_is_scheduled(task_t task)
{
	//INT_Disable();
	hw_disable_interrupts();
	uint8_t task_id = get_task_id(task);
	bool retVal = false;
	if(task_id != NO_TASK)
		retVal = is_scheduled(task_id);
	//INT_Enable();
	hw_enable_interrupts();
	return retVal;
}

error_t sched_post_task_prio(task_t task, unsigned int priority)
{
	error_t retVal;
	//INT_Disable();
	hw_disable_interrupts();
	check_structs_are_valid();
	uint8_t task_id = get_task_id(task);
	if(task_id == NO_TASK)
		retVal = EINVAL;
	else if(priority > MIN_PRIORITY || priority < MAX_PRIORITY)
		retVal = FAIL;
	else if (is_scheduled(task_id))
		retVal = EALREADY;
	else
	{
		if(NG(m_head)[priority] == NO_TASK)
		{
			NG(m_head)[priority] = task_id;
			NG(m_tail)[priority] = task_id;
		}
		else
		{
			NG(m_info)[NG(m_tail)[priority]].next = task_id;
			NG(m_info)[task_id].prev = NG(m_tail)[priority];
			NG(m_tail)[priority] = task_id;
		}
		NG(m_info)[task_id].priority = priority;
		//if our priority is higher than the currently known maximum priority
		if((priority < NG(current_priority)))
			NG(current_priority) = priority;
		check_structs_are_valid();
		retVal = SUCCESS;
	}
	//INT_Enable();
	hw_enable_interrupts();
	check_structs_are_valid();
	return retVal;
}

error_t sched_cancel_task(task_t task)
{
	check_structs_are_valid();
	error_t retVal;

	//INT_Disable();
	hw_disable_interrupts();
	uint8_t id = get_task_id(task);
	if(id == NO_TASK)
		retVal = EINVAL;
	else if(!is_scheduled(id))
		retVal = EALREADY;
	else
	{
		if (NG(m_info)[id].prev == NO_TASK)
			NG(m_head)[NG(m_info)[id].priority] = NG(m_info)[id].next;
		else
			NG(m_info)[NG(m_info)[id].prev].next = NG(m_info)[id].next;

		if (NG(m_info)[id].next == NO_TASK)
			NG(m_tail)[NG(m_info)[id].priority] = NG(m_info)[id].prev;
		else
			NG(m_info)[NG(m_info)[id].next].prev = NG(m_info)[id].prev;

		NG(m_info)[id].prev = NO_TASK;
		NG(m_info)[id].next = NO_TASK;
		NG(m_info)[id].priority = NOT_SCHEDULED;
		check_structs_are_valid();
		retVal = SUCCESS;
	}
	//INT_Enable();
	hw_enable_interrupts();
	return retVal;
}

static uint8_t pop_task(int priority)
{
	uint8_t id = NO_TASK;
	check_structs_are_valid();
	//assert(priority>=0 && priority < NUM_TASKS);
	//INT_Disable();
	hw_disable_interrupts();
	if (NG(m_head)[priority] != NO_TASK)
	{
		id = NG(m_head)[priority];
		NG(m_head)[priority] = NG(m_info)[NG(m_head)[priority]].next;
		if(NG(m_head)[priority] == NO_TASK)
			NG(m_tail)[priority] = NO_TASK;
		else
			NG(m_info)[NG(m_head)[priority]].prev = NO_TASK;

		NG(m_info)[id].next = NO_TASK;
		NG(m_info)[id].prev = NO_TASK;
		NG(m_info)[id].priority = NOT_SCHEDULED;
	}
	//INT_Enable();
	hw_enable_interrupts();
	check_structs_are_valid();
	return id;
}

static inline bool tasks_waiting(int priority)
{
	return NG(m_head)[priority] != NO_TASK;
}

void scheduler_run()
{
	while(1)
	{
		while(NG(current_priority) < NUM_PRIORITIES)
		{
			check_structs_are_valid();
			for(uint8_t id = pop_task((NG(current_priority))); id != NO_TASK; id = pop_task(NG(current_priority)))
			{
				check_structs_are_valid();
				NG(m_info)[id].task();
			}
			//this needs to be done atomically since otherwise we risk decrementing the current priority
			//while a higher priority task is waiting in the queues
			//INT_Disable();
			hw_disable_interrupts();
			if (!tasks_waiting(NG(current_priority)))
				NG(current_priority)++;
#ifndef NDEBUG
			for(int i = 0; i < NG(current_priority); i++)
				assert(!tasks_waiting(i));
#endif
			//INT_Enable();
			hw_enable_interrupts();
		}

		//system_lowpower_mode(0,1);
	}

}
