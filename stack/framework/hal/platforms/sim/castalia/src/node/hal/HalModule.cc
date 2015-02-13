/*
 * HalModule.cc
 *
 *  Created on: 12 Feb 2015
 *      Author: guust
 */

#include "HalModule.h"
#include "CRadio.h"
#include "ng.h"
#include <assert.h>
#include <cmath>

#include "bootstrap.h"
Define_Module(HalModule);

namespace
{

	enum
	{
		SCHEDULER_ID = 0,
		FRAMEWORK_TIMER_ID = 1,
		HW_TIMER_OFFSET = 2,
		NUM_HW_TICKS = 1<<(8*sizeof(hwtimer_tick_t)),
		NUM_FRTIMER_TICKS = 1<<(8*sizeof(timer_tick_t)),
		NUM_PRIORITIES = MIN_PRIORITY+1,
		NOT_SCHEDULED = NUM_PRIORITIES,
	};

	static inline int toCounterId(hwtimer_id_t id){ return id*2 + HW_TIMER_OFFSET;}
	static inline int toOverflowId(hwtimer_id_t id){return (id*2+1) + HW_TIMER_OFFSET;}

}

HalModule::HalModule():interTaskDelay(),taskExecutionTime(),cur_task_priority(NUM_PRIORITIES),scheduler_inited(false),
		frtimer_tick_length(),frtimer_reset_offset(),frtimer_inited(),hwtimers(),
		task_queue(),tasks(),frtimers(),frtimer_tasks()
{
}

HalModule::~HalModule()
{
}

bool HalModule::isContextValid()
{
	return activeModule == this && get_node_global_id() == self;
}


error_t HalModule::create_hwtimer(hwtimer_id_t id, simtime_t tick_length, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
	assert(isContextValid());

	if(id > HW_TIMER_OFFSET)
		return ESIZE;

	if(hwtimers.find(id) == hwtimers.end())
	{
		hwtimers[id].compare_callback = compare_callback;
		hwtimers[id].overflow_callback = overflow_callback;
		hwtimers[id].tick_length = tick_length;
		hw_timer_counter_reset(id);
		return SUCCESS;
	}
	else
		return EALREADY;
}

hwtimer_tick_t HalModule::get_hw_timer_tick(hw_timer_info const& info)
{
	unsigned int raw_ticks = std::floor((getClock()-info.reset_offset)/info.tick_length);
	return raw_ticks % NUM_HW_TICKS;
}

hwtimer_tick_t HalModule::hw_timer_getvalue(hwtimer_id_t timer_id)
{
	assert(isContextValid());
	std::map<hwtimer_id_t, hw_timer_info>::const_iterator timer = hwtimers.find(timer_id);

	if(timer == hwtimers.end())
		return 0;
	else
		return get_hw_timer_tick(timer->second);
}

error_t HalModule::hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t tick )
{
	assert(isContextValid());
	if(timer_id > HWTIMER_NUM)
		return ESIZE;

	std::map<hwtimer_id_t, hw_timer_info>::iterator timer = hwtimers.find(timer_id);
	if(timer == hwtimers.end())
		return EOFF;

	//don't even bother to set the timer if the callback value is 0x0
	if(timer->second.compare_callback == 0x0)
		return SUCCESS;

	cancelTimer(toCounterId(timer_id));

	//the current counter value
	hwtimer_tick_t cur_tick = get_hw_timer_tick(timer->second);
	//the actual time at which the current clock tick started
	simtime_t cur_tick_start_time = timer->second.reset_offset + cur_tick*timer->second.tick_length;

	//the total ticks to delay, starting from the current clock tick
	hwtimer_tick_t delay_ticks;

	if(cur_tick < tick)
		delay_ticks = tick - cur_tick;
	else
		delay_ticks = NUM_HW_TICKS - (cur_tick-tick);

	//the time to delay starting from the beginning of the current clock tick
	//the 0.001 is there offset minor rounding errors so
	//hw_timer_getvalue() reports the correct clocktick when the callback occurs
	simtime_t delay_time = (delay_ticks + 0.001)*timer->second.tick_length;
	//the timer should fire delay_time from the start of the current clock tick --> compensate for this
	delay_time-= (getClock()-cur_tick_start_time);

	setTimer(toCounterId(timer_id), delay_time);
//	std::cout << "Scheduling timer to go off at: " << (getClock() + delay_time) << " "
//			  << "expected timer tick: " << (((uint32_t)std::floor((getClock()+delay_time-timer->second.reset_offset)/timer->second.tick_length)) % NUM_HW_TICKS) << std::endl;
	return SUCCESS;
}
error_t HalModule::hw_timer_cancel(hwtimer_id_t timer_id)
{
	assert(isContextValid());
	if(timer_id > HWTIMER_NUM)
		return ESIZE;

	std::map<hwtimer_id_t, hw_timer_info>::iterator timer = hwtimers.find(timer_id);
	if(timer == hwtimers.end())
		return EOFF;

	cancelTimer(toCounterId(timer_id));
	return SUCCESS;
}

error_t HalModule::hw_timer_counter_reset(hwtimer_id_t timer_id)
{
	assert(isContextValid());
	if(timer_id > HWTIMER_NUM)
		return ESIZE;

	std::map<hwtimer_id_t, hw_timer_info>::iterator timer = hwtimers.find(timer_id);
	if(timer == hwtimers.end())
			return EOFF;

	cancelTimer(toCounterId(timer_id));
	cancelTimer(toOverflowId(timer_id));
	timer->second.reset_offset = getClock();

	if(timer->second.overflow_callback != 0x0)
		setTimer(toOverflowId(timer_id), NUM_HW_TICKS*timer->second.tick_length);

	return SUCCESS;
}

void HalModule::scheduler_init()
{
	assert(isContextValid());
	if(scheduler_inited)
		opp_error("Something went badly wrong: scheduler_init() called twice for node %d", self);
	scheduler_inited = true;
}
error_t HalModule::sched_register_task(task_t task)
{
	assert(isContextValid());
	if(tasks.find(task) != tasks.end())
		return EALREADY;
	else
	{
		tasks[task].priority = NOT_SCHEDULED;
		tasks[task].pos = task_queue[MIN_PRIORITY].end();
		return SUCCESS;
	}
}
error_t HalModule::sched_post_task_prio(task_t task, uint8_t priority)
{
	assert(isContextValid());
	if(tasks.find(task) == tasks.end())
		return EINVAL;
	else if(priority < MAX_PRIORITY || priority > MIN_PRIORITY)
		return ESIZE;
	else if(tasks[task].priority != NOT_SCHEDULED)
		return EALREADY;

	tasks[task].priority = priority;
	task_queue[priority].push_back(task);
	tasks[task].pos = --task_queue[priority].end();
	if(priority < cur_task_priority)
		cur_task_priority = priority;

	if(!isTimerScheduled(SCHEDULER_ID))
		setTimer(SCHEDULER_ID, interTaskDelay);

	return SUCCESS;
}
error_t HalModule::sched_cancel_task(task_t task)
{
	assert(isContextValid());
	if(tasks.find(task) == tasks.end())
		return EINVAL;
	else if (tasks[task].priority == NOT_SCHEDULED)
		return EALREADY;

	task_queue[tasks[task].priority].erase(tasks[task].pos);
	tasks[task].priority = NOT_SCHEDULED;
	tasks[task].pos = task_queue[MIN_PRIORITY].end();

	return SUCCESS;
}
bool HalModule::sched_is_scheduled(task_t task)
{
	assert(isContextValid());
	return (tasks.find(task) != tasks.end()) && tasks[task].priority != NOT_SCHEDULED;
}

HalModule::frtimer_info::frtimer_info(simtime_t const& fire_time, task_t const& task, uint8_t priority):fire_time(fire_time), task(task), priority(priority)
{}


void HalModule::timer_init()
{
	assert(isContextValid());
	if(frtimer_inited)
		opp_error("Something went badly wrong: timer_init() called twice for node %d", self);
	frtimer_inited = true;
	frtimer_reset_offset = getClock();
}
timer_tick_t HalModule::timer_get_counter_value()
{
	assert(isContextValid());
//	std::cout << "getClock(): " << getClock() << "\t"
//			  << "frtimer_reset_offset: " << frtimer_reset_offset << "\t"
//			  << "frtimer_tick_length: " << frtimer_tick_length << std::endl;
	return (timer_tick_t)std::floor((getClock() - frtimer_reset_offset)/frtimer_tick_length);
}
error_t HalModule::timer_post_task_prio(task_t task, timer_tick_t time, uint8_t priority)
{
	assert(isContextValid());
	if(priority < MAX_PRIORITY || priority > MIN_PRIORITY)
		return EINVAL;
	else if (frtimer_tasks.find(task) != frtimer_tasks.end())
		return EALREADY;

	reset_frtimer();

	//use signed expressions to determine whether 'time' is before or after the current time
	//(a trick borrowed from AODV: https://www.ietf.org/rfc/rfc3561.txt (page 11)
	if ( ((int32_t)time)-((int32_t)timer_get_counter_value()) < 0)
	{
		//event scheduled to the past --> fire it immediately
		sched_post_task_prio(task, priority);
	}
	else
	{
		simtime_t fire_time = getClock() + (time - timer_get_counter_value())*frtimer_tick_length;

		frtimer_tasks.insert(task);
		frtimers.insert(frtimer_info(fire_time, task, priority));
		configure_next_frtimer();
	}
	return SUCCESS;
}

error_t HalModule::timer_cancel_task(task_t task)
{
	assert(isContextValid());
	if(frtimer_tasks.find(task) == frtimer_tasks.end())
		return EALREADY;
	else
	{
		frtimer_tasks.erase(task);
		for(std::set<frtimer_info>::const_iterator i = frtimers.begin(); i != frtimers.end(); i++)
		{
			if(i->task == task)
			{
				frtimers.erase(i);
				break;
			}
		}
		configure_next_frtimer();
		return SUCCESS;
	}
}

bool operator<(HalModule::frtimer_info const& a, HalModule::frtimer_info const& b)
{
	return (a.fire_time != b.fire_time) ? (a.fire_time < b.fire_time) : (a.task < b.task);
}

void HalModule::reset_frtimer()
{
#ifdef FRAMEWORK_TIMER_RESET_COUNTER
	frtimer_reset_offset = getClock();
#endif
}
void HalModule::configure_next_frtimer()
{
	reset_frtimer();
	cancelTimer(FRAMEWORK_TIMER_ID);
	std::set<frtimer_info>::const_iterator next_event = frtimers.begin();
	if(next_event != frtimers.end())
	{
		setTimer(FRAMEWORK_TIMER_ID, next_event->fire_time-getClock());
	}
}

HalModule* HalModule::getActiveModule()
{
	assert(activeModule != 0x0);
	HalModule* mod = check_and_cast<HalModule*>(activeModule);
	assert(mod->self== get_node_global_id());
	return mod;
}

void HalModule::startup()
{
	interTaskDelay = (double)par("interTaskDelay");
	taskExecutionTime = (double)par("taskExecutionTime");
	scheduler_inited = false;
	cur_task_priority = NUM_PRIORITIES;
	task_queue.clear();
	task_queue = std::vector<std::list<task_t> >(NUM_PRIORITIES, std::list<task_t>());
	tasks.clear();

	frtimer_tick_length = 1.0/TIMER_TICKS_PER_SEC;
	frtimer_reset_offset = 0;
	frtimer_inited = false;
	frtimers.clear();
	frtimer_tasks.clear();
	hwtimers.clear();

	__framework_bootstrap();

}

void HalModule::timerFiredCallback(int index)
{
	switch(index)
	{
		case SCHEDULER_ID:
		{
			task_t task = 0x0;
			//scan all queues until a task has been found
			for(;cur_task_priority < NUM_PRIORITIES; cur_task_priority++)
			{
				if(!task_queue[cur_task_priority].empty())
				{
					task = (*task_queue[cur_task_priority].begin());
					task_queue[cur_task_priority].pop_front();
					break;
				}
			}
			//if one has been found --> clear it's status and execute it
			if(task != 0x0)
			{
				tasks[task].priority = NOT_SCHEDULED;
				tasks[task].pos = task_queue[MIN_PRIORITY].end();

				setTimer(SCHEDULER_ID, interTaskDelay + taskExecutionTime);
				task();
			}
			break;
		}
		case FRAMEWORK_TIMER_ID:
		{
			assert(!frtimers.empty());
			sched_post_task_prio(frtimers.begin()->task, frtimers.begin()->priority);
			frtimer_tasks.erase(frtimers.begin()->task);
			frtimers.erase(frtimers.begin());
			configure_next_frtimer();
			break;
		}
		default:
		{
			assert(index >=HW_TIMER_OFFSET && (index/2-HW_TIMER_OFFSET) < HWTIMER_NUM);

			hwtimer_id_t timer_id = (index-HW_TIMER_OFFSET)/2;
			std::map<hwtimer_id_t, hw_timer_info>::iterator timer = hwtimers.find(timer_id);
			assert(timer != hwtimers.end());
			if(toCounterId(timer_id) == index)
			{
				assert(timer->second.compare_callback != 0x0);
				timer->second.compare_callback();
			}
			else if(toOverflowId(timer_id) == index)
			{
				assert(timer->second.overflow_callback != 0x0);
				//update the reset_offset to ensure that hw_timer_getvalue(...) returns 0
				//if we don't do this tiny rounding errors can cause '65535' to be reported as the current time
				timer->second.reset_offset = getClock();
				setTimer(toOverflowId(timer_id), NUM_HW_TICKS*timer->second.tick_length);
				timer->second.overflow_callback();
			}
		}
	}
}

void HalModule::finishSpecific()
{
	activeModule = 0x0;
}
void HalModule::fromRadio(RadioPacket* packet)
{

}
void HalModule::handleRadioControlMessage(RadioControlMessage* msg)
{

}
