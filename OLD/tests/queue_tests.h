#include "test.h"

#include "../d7aoss/framework/queue.h"

void queue_when_pushing_value_and_queue_is_full_it_should_return_false(void **state)
{
    int size = 10;
    char buffer[size];
    queue_t q;
    queue_init(&q, buffer, size);
    int max_queue_len = sizeof(buffer) / sizeof(uint8_t);
    uint8_t i;
    for(i = 0; i < max_queue_len; i++)
        assert_true(queue_push_value(&q, &i, sizeof(i)));

    assert_false(queue_push_value(&q, &i, sizeof(i)));
}

const UnitTest test_queue_tests[] = {
    unit_test(queue_when_pushing_value_and_queue_is_full_it_should_return_false),
};
