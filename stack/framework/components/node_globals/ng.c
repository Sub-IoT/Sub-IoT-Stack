#include "ng.h"
#if defined(NODE_GLOBALS)
size_t __nc_node_id__ = 0xFFFFFFFF;
__LINK_C void set_node_global_id(size_t node_id)
{
	assert(node_id < __nc_max_nodes__);
    __nc_node_id__ = node_id;
}
#endif
