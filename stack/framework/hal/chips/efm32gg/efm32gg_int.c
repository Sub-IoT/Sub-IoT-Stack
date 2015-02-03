#include "hwint.h"
#include "em_int.h"

void hw_disable_interrupts()
{
    INT_Disable();
}

void hw_enable_interrupts()
{
    INT_Enable();
}