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
#include "stdio.h"
#include <stdlib.h>
#include "timer.c"

#define COUNTER_OVERFLOW_MAX 0xFFFF0000

static timer_event dummy_tester_event;
static timer_event dummy_tester_event2;
static timer_event dummy_tester_event3;
static timer_event dummy_tester_event4;
static timer_event dummy_tester_event5;
static timer_event dummy_tester_event6;

void dummy_function(){}
void dummy_function2(){}
void dummy_function3(){}
void dummy_function4(){}
void dummy_function5(){}
void dummy_function6(){}
void dummy_function7(){}
void dummy_function8(){}

void test_timer_events() {
    //can init event
    assert(timer_init_event(&dummy_tester_event, &dummy_function) == SUCCESS);
    //cannot init same event multiple times
    assert(timer_init_event(&dummy_tester_event, &dummy_function) == -EALREADY);
    //manually conf next_event
    dummy_tester_event.next_event = TIMER_TICKS_PER_SEC * 1;
    // can post a task from an event
    assert(timer_add_event(&dummy_tester_event) == SUCCESS);
    // after adding event, associated task is scheduled
    assert(timer_is_task_scheduled(dummy_tester_event.f));
    // cancel event
    timer_cancel_event(&dummy_tester_event);
    // after cancelling event, associated task is no longer scheduled
    assert(timer_is_task_scheduled(dummy_tester_event.f) == false);
}

void test_timer_post_tasks() {
    // cannot post task with less than min priority
    assert(timer_post_task_prio_delay(&dummy_function2, TIMER_TICKS_PER_SEC * 5, MIN_PRIORITY + 1) == EINVAL);
    // can post task 
    assert(timer_post_task_prio_delay(&dummy_function2, TIMER_TICKS_PER_SEC * 5, DEFAULT_PRIORITY) == SUCCESS);
    // after posting, task is scheduled
    assert(timer_is_task_scheduled(&dummy_function2));
    // can modify the fire time by posting the same task again
    assert(timer_post_task_prio_delay(&dummy_function2, TIMER_TICKS_PER_SEC * 7, DEFAULT_PRIORITY) == SUCCESS);
    // cannot modify the priority by posting the same task again
    assert(timer_post_task_prio_delay(&dummy_function2, TIMER_TICKS_PER_SEC * 7, DEFAULT_PRIORITY-1) == EALREADY);
    // can cancel the task
    assert(timer_cancel_task(&dummy_function2) == SUCCESS);
    // after cancelling, task is no longer scheduled
    assert(timer_is_task_scheduled(&dummy_function2) == false);
    // cannot cancel an unscheduled task
    assert(timer_cancel_task(&dummy_function2) == EALREADY);

    // task posted with delay of 0 is scheduled immediately
    sched_register_task(&dummy_function3);
    assert(timer_post_task_prio_delay(&dummy_function3, TIMER_TICKS_PER_SEC * 0, DEFAULT_PRIORITY) == SUCCESS);
    assert(timer_is_task_scheduled(&dummy_function3) == false);
}

void run_overflow_test(timer_tick_t timer_offset, timer_tick_t next_event_time) {
    NG(timer_offset) = timer_offset;
    NG(timers)[NG(next_event)].next_event = next_event_time;
    NG(hw_event_scheduled) = false;
    run_overflow_c(); 
}
void test_timer_overflow() {
    set_hw_timer_value(0);
    //regular cases:
    //create an event
    assert(timer_init_event(&dummy_tester_event2, &dummy_function4) == SUCCESS);
    dummy_tester_event2.next_event = TIMER_TICKS_PER_SEC * 1;
    assert(timer_add_event(&dummy_tester_event2) == SUCCESS);

    // event is in the next period
    run_overflow_test(COUNTER_OVERFLOW_INCREASE * 10, COUNTER_OVERFLOW_INCREASE * 11 + 3000); 
    assert(NG(hw_event_scheduled) == true); //i.e. event gets scheduled for the next period
    
    // event is in the far future
    run_overflow_test(COUNTER_OVERFLOW_INCREASE * 10, COUNTER_OVERFLOW_INCREASE * 12 + 3000); 
    assert( (NG(hw_event_scheduled) == false) && NG(timers)[NG(next_event)].f != 0x0 ); //i.e. event should not be scheduled yet

    // event is in the distant past
    run_overflow_test(COUNTER_OVERFLOW_INCREASE * 10, 0); 
    assert( (NG(hw_event_scheduled) == false) && NG(timers)[NG(next_event)].f != 0x0 ); //i.e. event doesn't get fired
    
    // event is in this period but in near past (behind hw_timer time)
    set_hw_timer_value(4000);
    run_overflow_test(COUNTER_OVERFLOW_INCREASE * 10, COUNTER_OVERFLOW_INCREASE * 11 + 3000); 
    assert( (NG(hw_event_scheduled) == false) && get_next_event() == NO_EVENT ); //i.e. event got fired immediately
    set_hw_timer_value(0);
    
    timer_cancel_event(&dummy_tester_event2);

    //overflow cases:    
    //create another event
    assert(timer_init_event(&dummy_tester_event3, &dummy_function5) == SUCCESS);
    dummy_tester_event3.next_event = TIMER_TICKS_PER_SEC * 1;
    assert(timer_add_event(&dummy_tester_event3) == SUCCESS);

    // event is in the next period
    run_overflow_test(COUNTER_OVERFLOW_MAX, 3000); 
    assert(NG(hw_event_scheduled) == true); //i.e. event gets scheduled for the next period

    // event is in the far future
    run_overflow_test(COUNTER_OVERFLOW_MAX, COUNTER_OVERFLOW_INCREASE + 3000); 
    assert( (NG(hw_event_scheduled) == false) && NG(timers)[NG(next_event)].f != 0x0 ); //i.e. event should not be scheduled yet
   
    // event is in the distant past
    set_hw_timer_value(0);
    run_overflow_test(0, COUNTER_OVERFLOW_MAX); 
    assert( (NG(hw_event_scheduled) == false) && NG(timers)[NG(next_event)].f != 0x0 ); //i.e. event doesn't get fired

    // event is in this period but in near past (behind hw_timer time)
    set_hw_timer_value(4000);
    run_overflow_test(COUNTER_OVERFLOW_MAX, 3000); 
    assert( (NG(hw_event_scheduled) == false) && get_next_event() == NO_EVENT ); //i.e. event got fired immediately

    timer_cancel_event(&dummy_tester_event3);
}

void setup_counter_test(timer_tick_t timer_offset, uint16_t hw_timer_value, bool is_overflow_pending) {
    NG(timer_offset) = timer_offset;
    set_hw_timer_value(hw_timer_value);
    set_overflow_pending(is_overflow_pending);
}

void test_timer_get_counter_value() {
    //regular cases:
    setup_counter_test(COUNTER_OVERFLOW_INCREASE, 0x00ff, false);
    assert(timer_get_counter_value() == COUNTER_OVERFLOW_INCREASE + 0x00ff);
    setup_counter_test(COUNTER_OVERFLOW_INCREASE, 0x00ff, true);
    assert(timer_get_counter_value() == COUNTER_OVERFLOW_INCREASE + 0x00ff + COUNTER_OVERFLOW_INCREASE);

    //overflow cases:
    setup_counter_test(COUNTER_OVERFLOW_MAX, 0x00ff, false);
    assert(timer_get_counter_value() == COUNTER_OVERFLOW_MAX + 0x00ff);
    setup_counter_test(COUNTER_OVERFLOW_MAX, 0x00ff, true);
    assert(timer_get_counter_value() == COUNTER_OVERFLOW_MAX + 0x00ff + COUNTER_OVERFLOW_INCREASE);
}

void test_timer_timed_events() {
    //reset time to 0
    NG(timer_offset) = 0;
    set_hw_timer_value(0);
    set_overflow_pending(false);

    // - schedule three events of different times
    assert(timer_init_event(&dummy_tester_event4, &dummy_function6) == SUCCESS);
    dummy_tester_event4.next_event = TIMER_TICKS_PER_SEC * 1;
    assert(timer_init_event(&dummy_tester_event5, &dummy_function7) == SUCCESS);
    dummy_tester_event5.next_event = TIMER_TICKS_PER_SEC * 3;
    assert(timer_init_event(&dummy_tester_event6, &dummy_function8) == SUCCESS);
    dummy_tester_event6.next_event = TIMER_TICKS_PER_SEC * 5;
    
    // post tasks from the events
    assert(timer_add_event(&dummy_tester_event4) == SUCCESS);
    assert(timer_add_event(&dummy_tester_event5) == SUCCESS);
    assert(timer_add_event(&dummy_tester_event6) == SUCCESS);
    
    assert(timer_is_task_scheduled(dummy_tester_event4.f));
    // check event returned is as expected
    uint32_t next_event = get_next_event();
    assert(NG(timers)[next_event].f == dummy_tester_event4.f); 
    // move time forward till after that event
    NG(timer_offset) = TIMER_TICKS_PER_SEC * 2;
    // events from the past should still get returned if they haven't fired yet
    next_event = get_next_event();
    assert(NG(timers)[next_event].f == dummy_tester_event4.f);
    timer_cancel_event(&dummy_tester_event4);
    next_event = get_next_event();
    assert(NG(timers)[next_event].f == dummy_tester_event5.f);
    timer_cancel_event(&dummy_tester_event5);
    next_event = get_next_event();
    assert(NG(timers)[next_event].f == dummy_tester_event6.f);
    timer_cancel_event(&dummy_tester_event6);
    //when no events listed, should return NO_EVENT
    next_event = get_next_event();
    assert(next_event == NO_EVENT);
}

int main(int argc, char *argv[]){
    timer_init();
    scheduler_init();

    test_timer_events();
    
    test_timer_post_tasks();
    
    test_timer_overflow();
    
    test_timer_get_counter_value();
    
    test_timer_timed_events();

    printf("All timer tests passed!\n");

    exit(0);
}