#include "hwspi.h"
#include <assert.h>
__LINK_C void spi_init() { assert(false); }
__LINK_C void spi_auto_cs_on(void) { assert(false); }
__LINK_C void spi_auto_cs_off(void) { assert(false); }
__LINK_C void spi_select_chip(void) { assert(false); }
__LINK_C void spi_deselect_chip(void) { assert(false); }
__LINK_C uint8_t spi_byte(uint8_t) { assert(false); return 0;}
__LINK_C void spi_string(uint8_t *, uint8_t *, size_t) { assert(false); }

