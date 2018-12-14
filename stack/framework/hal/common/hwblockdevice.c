
#include "hwblockdevice.h"
#include "debug.h"

void blockdevice_init(blockdevice_t* bd) {
  assert(bd && bd->driver && bd->driver->init);
  bd->driver->init(bd);
}

error_t blockdevice_read(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size) {
  assert(bd && bd->driver && bd->driver->read);
  return bd->driver->read(bd, data, addr, size);
}

error_t blockdevice_program(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size) {
  assert(bd && bd->driver && bd->driver->program);
  return bd->driver->program(bd, data, addr, size);
}

