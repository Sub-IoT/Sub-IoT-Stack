/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* \file
 *
 * A Collection of helper functions to help debug applications running in the simulator
 *
 */
#ifndef __SIM_UTILS_H_
#define __SIM_UTILS_H_

#include "link_c.h"

/* \brief Get the number of seconds that has passed in 'simulation time' 
 * according to the clock of the node itself. This will be different for different
 * nodes because of the (simulated) timerdrift.
 *
 * \return double	The current simulation time according to the clock of the node
 */
__LINK_C double get_sim_node_time();

/* \brief Get the number of seconds that has passed in 'simulation time' 
 * according to the global clock. This neglects timer drift between different nodes
 * and is therefore the same regardless of which node queries it.
 * \return double	The current simulation time according to the global sim clock
 */
__LINK_C double get_sim_real_time();


#endif // __SIM_UTILS_H_
