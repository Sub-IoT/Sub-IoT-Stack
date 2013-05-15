#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmockery.h>
#include <unistd.h>

#include <assert.h>

#include "../d7aoss/framework/timer.h"

// If unit testing is enabled override assert with mock_assert().
#define UNIT_TESTING // TODO define using cmake
#ifdef UNIT_TESTING
extern void mock_assert(const int result, const char* const expression,
                        const char * const file, const int line);
#undef assert
#define assert(expression) \
    mock_assert((int)(expression), #expression, __FILE__, __LINE__);
#endif // UNIT_TESTING

bool callback_called = false;

void callback()
{
    callback_called = true;
}

void timer_when_adding_event_callback_should_be_called(void **state)
{
    timer_init();
    timer_event t;
    t.f = &callback;
    t.next_event = 5;
    timer_add_event(&t);

    pause();

    assert(callback_called);
}

int main(int argc, char* argv[]) {
    const UnitTest tests[] = {
        unit_test(timer_when_adding_event_callback_should_be_called),
    };

    return run_tests(tests);
}
