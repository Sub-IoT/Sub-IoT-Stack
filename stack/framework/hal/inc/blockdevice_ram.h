
#include "hwblockdevice.h"
#include "hwgpio.h"


// This is a blockdevice implementation using a buffer in RAM, mainly used as fallback for platforms which do not have
// a non-volatile memory (driver)

// extend blockdevice_t
typedef struct {
  blockdevice_t base;
  uint32_t size;
  uint8_t* buffer;
} blockdevice_ram_t;

extern blockdevice_driver_t blockdevice_driver_ram;
