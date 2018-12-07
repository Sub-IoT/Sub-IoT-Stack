

#ifndef BLOCKDEVICE_H_
#define BLOCKDEVICE_H_

#include "types.h"
#include "errors.h"

// TODO generalize to allow different drivers
void blockdevice_init();
error_t blockdevice_read(uint8_t* data, uint32_t addr, uint32_t size);
error_t blockdevice_program(uint8_t* data, uint32_t addr, uint32_t size);

#endif

