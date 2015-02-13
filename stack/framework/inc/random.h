/* \file
 *
 * This file specifies the random number generator facilities of the framework
 *
 */
#ifndef __RANDOM_H_
#define __RANDOM_H_

#include "types.h"
#include "link_c.h"

/*! \brief Get a random number.
 * 
 * The exact implementation of the random number generator is platform specific. On (real)
 * hardware this call most likely forwards to the rand() function of the C-library.
 * In a simulation environment however, this call forwards to a random number generator of the
 * simulator itself to ensure that tests are repeatable
 *
 * \return uint32_t	a semi-random number between 0 and 2^32-1
 */
__LINK_C uint32_t get_rnd();

/*! Set the seed for the random nuber generator
 *
 * \param	seed	The seed for the random number generator
 *
 */
__LINK_C void set_rng_seed(unsigned int seed);


#endif // __RANDOM_H_

