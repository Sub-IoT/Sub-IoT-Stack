
#include "stm32_common_eeprom.h"
#include "stm32_device.h"
#include "debug.h"
#include "string.h"

// TODO validate embedded EEPROM works the same on stm32l1 and uses same start address and size as well

// forward declare driver function pointers
static void init(blockdevice_t* bd);
static error_t read(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size);
static error_t program(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size);

blockdevice_driver_t blockdevice_driver_stm32_eeprom = {
    .init = init,
    .read = read,
    .program = program,
};


static void init(blockdevice_t* bd) { // TODO SPI as param
  (void)bd; // suppress unused warning
}

static error_t read(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size) {
  (void)bd; // suppress unused warning

  if(size == 0) return SUCCESS;

  addr += DATA_EEPROM_BASE;
  if(addr + size > DATA_EEPROM_BANK2_END) return -ESIZE;

  memcpy(data, (const void*)(addr), size);

  return SUCCESS;
}

static error_t program(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size) {
  (void)bd; // suppress unused warning

  if(size == 0) return SUCCESS;

  addr += DATA_EEPROM_BASE;
  if(addr + size > DATA_EEPROM_BANK2_END) return -ESIZE;

  // TODO optimize by writing per word when possible
  HAL_FLASHEx_DATAEEPROM_Unlock();
  for (size_t i = 0; i < size; i++) {
    while (FLASH->SR & FLASH_SR_BSY); // TODO timeout
    *(uint8_t*)(addr + i) = data[i];
    while (FLASH->SR & FLASH_SR_BSY); // TODO timeout
  }

  HAL_FLASHEx_DATAEEPROM_Lock();

  return SUCCESS;
}
