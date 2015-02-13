#include "random.h"
#include "types.h"
#include <omnetpp.h>
uint32_t get_rnd()
{
    //return an integer in the range 0 -> 0xFFFFFFFF
    return intrand(0xFFFFFFFF);
}

void set_rng_seed(unsigned int seed)
{
    //ignore this call
}
