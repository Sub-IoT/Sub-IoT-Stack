
#include "hwblockdevice.h"
#include "hwgpio.h"

// extend blockdevice_t
typedef struct {
  blockdevice_t base;
} blockdevice_stm32_eeprom_t;

extern blockdevice_driver_t blockdevice_driver_stm32_eeprom;
