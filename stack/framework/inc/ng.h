#ifndef NG_H_
#define NG_H_
#if defined(NODE_GLOBALS)
#include <stddef.h>
#include <assert.h>
#ifndef NODE_GLOBALS_MAX_NODES
    #warning NODE_GLOBALS_MAX_NODES is not defined. Using default value of 256
    #define NODE_GLOBALS_MAX_NODES 256
#endif

enum
{
    __nc_max_nodes__ = NODE_GLIBALS_MAX_NODES,
};
extern size_t __nc_node_id__;
void set_node_global_id(size_t node_id);
static inline size_t get_node_global_id() { assert(__nc_node_id__ < __nc_max_nodes__); return __nc_node_id__; }

#define NG(var)			(__ng_glob_ ## var ## __[(get_node_global_id())])
#define NGDEF(var)		(__ng_glob_ ## var ## __[__nc_max_nodes__]);

#else

#define NGDEF(var)	(__ng_single_ ## var ## __)
#define NG(var)		(__ng_single_ ## var ## __)


#endif




#endif //NG_H_