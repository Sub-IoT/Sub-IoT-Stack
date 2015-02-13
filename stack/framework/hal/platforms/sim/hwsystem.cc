#include "hwsystem.h"
#include <assert.h>
#include "ng.h"

#ifndef NODE_GLOBALS
	#error NODE_GLOBALS MUST be defined when the simulator implementation of hwsystem is used
#endif

__LINK_C void hw_enter_lowpower_mode(uint8_t mode)
{
    assert(false);
}

__LINK_C uint64_t hw_get_unique_id()
{
	return get_node_global_id() +1 ;
}
