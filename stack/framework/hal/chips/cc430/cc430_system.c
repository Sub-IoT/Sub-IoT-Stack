#include "hwsystem.h"

#include <msp430.h>
#include <assert.h>

void hw_enter_lowpower_mode(uint8_t mode)
{
    switch(mode)
    {
        case 0:
        {
            __bis_SR_register(LPM0_bits | GIE);
            break;
        }
        case 1:
        {
            __bis_SR_register(LPM1_bits | GIE);
            break;
        }
        case 2:
        {
            __bis_SR_register(LPM2_bits | GIE);
            break;
        }
        case 3:
        {
            __bis_SR_register(LPM3_bits | GIE);
            break;
        }
        case 4:
        {
            __bis_SR_register(LPM4_bits | GIE);
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
