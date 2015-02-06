#include "random.h"
#include "types.h"
#include <stdlib.h>

uint32_t get_rnd()
{
    return (uint32_t) rand();
}

void set_rng_seed(unsigned int seed)
{
    srand(seed);
}
