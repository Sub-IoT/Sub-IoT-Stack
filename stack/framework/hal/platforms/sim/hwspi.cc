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

#include "hwspi.h"
#include <assert.h>
__LINK_C void spi_init() { assert(false); }
__LINK_C void spi_auto_cs_on(void) { assert(false); }
__LINK_C void spi_auto_cs_off(void) { assert(false); }
__LINK_C void spi_select_chip(void) { assert(false); }
__LINK_C void spi_deselect_chip(void) { assert(false); }
__LINK_C uint8_t spi_byte(uint8_t) { assert(false); return 0;}
__LINK_C void spi_string(uint8_t *, uint8_t *, size_t) { assert(false); }

