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

#ifndef NG_H_
#define NG_H_
#include "link_c.h"
//#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#if defined(NODE_GLOBALS)
#include <stddef.h>
#include <debug.h>
#ifndef NODE_GLOBALS_MAX_NODES
    #warning NODE_GLOBALS_MAX_NODES is not defined. Using default value of 256
    #define NODE_GLOBALS_MAX_NODES 256
#endif

enum
{
    __ng_max_nodes__ = NODE_GLOBALS_MAX_NODES,
};
extern size_t __ng_node_id__;
__LINK_C void set_node_global_id(size_t node_id);
static inline size_t get_node_global_id() { assert(__ng_node_id__ < __ng_max_nodes__); return __ng_node_id__; }

#define NG(var)			(__ng_glob_ ## var ## __[(get_node_global_id())])
#define NGDEF(var)		(__ng_glob_ ## var ## __[__ng_max_nodes__])

#else

#define NGDEF(var)	(__ng_single_ ## var ## __)
#define NG(var)		(__ng_single_ ## var ## __)


#endif //defined(NODE_GLOBALS)


#ifdef __cplusplus
}
#endif //__cplusplus


#endif //NG_H_
