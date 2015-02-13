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
