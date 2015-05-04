#include "test.h"

#include "timer_tests.h"
#include "queue_tests.h"

int main(int argc, char* argv[])
{
    return run_tests(test_timer_tests)
            || run_tests(test_queue_tests);
}
