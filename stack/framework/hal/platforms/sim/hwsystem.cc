#include "hwsystem.h"
#include <assert.h>

extern "C" void hw_enter_lowpower_mode(uint8_t mode)
{
    assert(false);
}

extern "C"uint64_t hw_get_unique_id()
{
    assert(false);
    return 0;
}
