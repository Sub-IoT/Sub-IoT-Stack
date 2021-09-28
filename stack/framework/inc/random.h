/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file random.h
 * \addtogroup random
 * \ingroup framework
 * @{
 * \brief Specifies the random number generator facilities of the framework
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
__LINK_C uint32_t get_rnd(void);

/*! \brief Set the seed for the random nuber generator
 *
 * \param	seed	The seed for the random number generator
 *
 */
__LINK_C void set_rng_seed(unsigned int seed);


#endif // __RANDOM_H_

/** @}*/
