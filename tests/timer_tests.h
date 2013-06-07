#include "test.h"

#include "../d7aoss/framework/timer.h"


bool callback_called = false;

static void callback()
{
    callback_called = true;
}

void test_timer_when_adding_event_callback_should_be_called(void **state)
{
    callback_called = false;

    timer_init();
    timer_event t;
    t.f = &callback;
    t.next_event = 5;
    timer_add_event(&t);

    pause();

    assert(callback_called);
}

void test_timer_when_adding_event_with_next_event_eq_0_callback_should_be_called_immediately(void **state)
{
    callback_called = false;

    timer_init();
    timer_event t;
    t.f = &callback;
    t.next_event = 0;
    timer_add_event(&t);

    // don't wait for signal which triggers callback;

    assert(callback_called);
}

void test_timer_when_adding_event_and_queue_is_full_it_should_return_false(void **state)
{
    int max_queue_len = sizeof(event_array) / sizeof(timer_event);
    timer_event t;
    t.f = &callback;
    t.next_event = 1000;
    int i;
    for(i = 0; i < max_queue_len; i++)
        timer_add_event(&t);

    assert_false(timer_add_event(&t));
}

const UnitTest test_timer_tests[] = {
    unit_test(test_timer_when_adding_event_callback_should_be_called),
    unit_test(test_timer_when_adding_event_with_next_event_eq_0_callback_should_be_called_immediately),
    unit_test(test_timer_when_adding_event_and_queue_is_full_it_should_return_false),
};


