/*
 *  Created on: May 9, 2013
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */

#include <timer.h>

void timer_init()
{
    _timer_init();

    queue_init(&event_queue, (u8*)&event_array, sizeof(event_array));
    started = false;
}

bool timer_insert_value_in_queue(timer_event* event)
{
    // TODO: substract time already gone
    u8 position = 0;
    int16_t sum_next_event = - _timer_getvalue();

    while (position < event_queue.length)
    {
        timer_event *temp_event = (timer_event*) queue_read_value(&event_queue, position, sizeof(timer_event));
        if (event->next_event > sum_next_event + temp_event->next_event)
        {
            sum_next_event += temp_event->next_event;
        } else {
            uint16_t elapsed = 0;
            if (position == 0)
            {
                elapsed = _timer_getvalue();
                _timer_disable_interrupt();
                started = false;
            } else {
                event->next_event -= sum_next_event;
            }

            queue_insert_value(&event_queue, (void*) event, position, sizeof(timer_event));
            temp_event = (timer_event*) queue_read_value(&event_queue, position+1, sizeof(timer_event));

            temp_event->next_event -= (event->next_event + elapsed);
            return true;
        }
        position++;
    }

    if (position == event_queue.length)
    {
        if (started) event->next_event -= sum_next_event;
        return queue_push_value(&event_queue, (void*) event, sizeof(timer_event));
    }

    return true;
}

bool timer_add_event(timer_event* event)
{
    if (event->next_event == 0)
    {
        event->f(NULL);
        return true;
    }

    timer_event new_event;
    new_event.f = event->f;
    new_event.next_event = event->next_event;

    if (timer_insert_value_in_queue(&new_event))
    {
        if (!started)
        {
            _timer_enable_interrupt();
            started = true;
            _timer_setvalue(event->next_event);
        }
    } else {
        log_print_string("Cannot add event, queue is full");
        return false;
    }

    return true;
}

void timer_completed()
{
    timer_event* event = (timer_event*) queue_pop_value(&event_queue, sizeof(timer_event));

    if (event_queue.length == 0)
    {
        _timer_disable_interrupt();
        started = false;
    } else {
        _timer_setvalue(((timer_event*) event_queue.front)->next_event);
    }

    event->f(NULL);
}
