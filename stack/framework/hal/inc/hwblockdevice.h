

#ifndef BLOCKDEVICE_H_
#define BLOCKDEVICE_H_

#include "types.h"
#include "errors.h"


typedef struct {
  void (*init)();
  error_t (*read)(uint8_t* data, uint32_t addr, uint32_t size);
  error_t (*program)(uint8_t* data, uint32_t addr, uint32_t size);
} blockdevice_driver_t;

void blockdevice_init(blockdevice_driver_t* driver);
error_t blockdevice_read(blockdevice_driver_t* driver, uint8_t* data, uint32_t addr, uint32_t size);
error_t blockdevice_program(blockdevice_driver_t* driver, uint8_t* data, uint32_t addr, uint32_t size);

#endif

