#include "ng.h"
#if defined(NODE_GLOBALS)
size_t __nc_node_id__ = 0xFFFFFFFF;
void set_node_global_id(size_t node_id)
{
    assert(node_id < MAX_NUM_NODES);
    __nc_node_id__ = node_id;
}
#endif
