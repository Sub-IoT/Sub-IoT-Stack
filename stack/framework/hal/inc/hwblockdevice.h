

#ifndef BLOCKDEVICE_H_
#define BLOCKDEVICE_H_

#include "types.h"
#include "errors.h"

typedef struct blockdevice blockdevice_t;

typedef struct {
  void (*init)(blockdevice_t* bd);
  error_t (*read)(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size);
  error_t (*program)(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size);
  error_t (*erase_chip)(blockdevice_t* bd);
  error_t (*erase_block32k)(blockdevice_t* bd, uint32_t addr);
  error_t (*erase_sector4k)(blockdevice_t* bd, uint32_t addr);
  uint32_t erase_block_size;
  uint32_t write_block_size;
} blockdevice_driver_t;

typedef error_t (*blockdevice_erase_t )(blockdevice_t* bd, uint32_t addr);

struct blockdevice {
  blockdevice_driver_t* driver;
};

void blockdevice_init(blockdevice_t* bd);
error_t blockdevice_read(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size);
error_t blockdevice_program(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size);
error_t blockdevice_erase_chip(blockdevice_t* bd, uint32_t addr);
error_t blockdevice_erase_block32k(blockdevice_t* bd, uint32_t addr);
error_t blockdevice_erase_sector4k(blockdevice_t* bd, uint32_t addr);

#endif

