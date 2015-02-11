#include "hwspi.h"
#include <assert.h>
extern "C" void spi_init()
{
    assert(false);
}
extern "C" void spi_auto_cs_on(void)
{
    assert(false);
}
extern "C" void spi_auto_cs_off(void)
{
    assert(false);
}
extern "C" void spi_select_chip(void)
{
    assert(false);
}
extern "C" void spi_deselect_chip(void)
{
    assert(false);
}
extern "C" uint8_t spi_byte(uint8_t)
{
    assert(false);
    return 0;
}
extern "C" void spi_string(uint8_t *, uint8_t *, size_t)
{
    assert(false);
}

