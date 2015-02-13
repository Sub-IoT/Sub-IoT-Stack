#include "sim_utils.h"
#include "HalModule.h"

__LINK_C double get_sim_node_time()
{
    return HalModule::getActiveModule()->get_node_time();
}
__LINK_C double get_sim_real_time()
{
    return HalModule::getActiveModule()->get_real_time();
}
