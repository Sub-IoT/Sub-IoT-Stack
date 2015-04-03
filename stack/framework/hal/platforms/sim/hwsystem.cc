//
// OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
// lowpower wireless sensor communication
//
// Copyright 2015 University of Antwerp
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "hwsystem.h"
#include <assert.h>
#include "ng.h"

#ifndef NODE_GLOBALS
	#error NODE_GLOBALS MUST be defined when the simulator implementation of hwsystem is used
#endif

__LINK_C void hw_enter_lowpower_mode(uint8_t mode)
{
    assert(false);
}

__LINK_C uint64_t hw_get_unique_id()
{
	return get_node_global_id() +1 ;
}
