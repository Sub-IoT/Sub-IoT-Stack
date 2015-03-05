#include "hwsystem.h"

#include <msp430.h>
#include <assert.h>

void hw_enter_lowpower_mode(uint8_t mode)
{
    switch(mode)
    {
        case 0:
        {
            LPM0;
            break;
        }
        case 1:
        {
            LPM1;
            break;
        }
        case 2:
        {
            LPM2;
            break;
        }
        case 3:
        {
            LPM3;
            break;
        }
        case 4:
        {
            LPM4;
            break;
        }
        default:
        {
            assert(0);
        }
    }
}

uint64_t hw_get_unique_id()
{
    // TODO
}
