
#include "hwblockdevice.h"
#include "debug.h"

void blockdevice_init(blockdevice_driver_t* driver) {
  assert(driver && driver->init);
  driver->init();
}

error_t blockdevice_read(blockdevice_driver_t* driver, uint8_t* data, uint32_t addr, uint32_t size) {
  assert(driver && driver->read);
  return driver->read(data, addr, size);
}

error_t blockdevice_program(blockdevice_driver_t* driver, uint8_t* data, uint32_t addr, uint32_t size) {
  assert(driver && driver->program);
  return driver->program(data, addr, size);
}

