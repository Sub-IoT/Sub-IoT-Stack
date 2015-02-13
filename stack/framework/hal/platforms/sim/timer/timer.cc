#include "timer.h"
#include "HalModule.h"

extern "C" void timer_init()
{
	HalModule::getActiveModule()->timer_init();
}
extern "C" timer_tick_t timer_get_counter_value()
{
	return HalModule::getActiveModule()->timer_get_counter_value();
}
extern "C" error_t timer_post_task_prio(task_t task, timer_tick_t time, uint8_t priority)
{
	return HalModule::getActiveModule()->timer_post_task_prio(task, time, priority);
}
extern "C" error_t timer_cancel_task(task_t task)
{
	return HalModule::getActiveModule()->timer_cancel_task(task);
}
