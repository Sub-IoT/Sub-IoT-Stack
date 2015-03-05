#include "hwatomic.h"
#include <msp430.h>

// TODO nested critical sections not yet supported

void start_atomic()
{
    __disable_interrupt();
}

void end_atomic()
{
    __enable_interrupt();
}
