#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include "platform_defs.h"

#ifndef PLATFORM_SIM
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_SIM to be defined
#endif

#define NODE_GLOBALS
#define NODE_GLOBALS_MAX_NODES PLATFORM_SIM_MAX_NODES

#define PLATFORM_NUM_TIMERS 127
#define HW_NUM_LEDS 0


#endif
