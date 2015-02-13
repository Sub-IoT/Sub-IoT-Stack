#include "random.h"
#include "types.h"
#include <stdlib.h>

__LINK_C uint32_t get_rnd()
{
    return (uint32_t) rand();
}

__LINK_C void set_rng_seed(unsigned int seed)
{
    srand(seed);
}
