
#include "hwblockdevice.h"
#include "hwgpio.h"

// concrete blockdevice implemetation for STM32 embedded EEPROM. Allows to divide the EEPROM in different blocks using offset and size
typedef struct {
  blockdevice_t base;
  uint32_t offset; // offset in EEPROM where the blockdevice starts
  uint32_t size;   // size of the blockdevice
} blockdevice_stm32_eeprom_t;

extern blockdevice_driver_t blockdevice_driver_stm32_eeprom;
