
#include "stm32_common_eeprom.h"
#include "stm32_device.h"
#include "debug.h"
#include "string.h"

// TODO validate embedded EEPROM works the same on stm32l1 and uses same start address and size as well

// forward declare driver function pointers
static void init(blockdevice_t* bd);
static error_t read(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size);
static error_t program(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size);

blockdevice_driver_t blockdevice_driver_stm32_eeprom = {
    .init = init,
    .read = read,
    .program = program,
    .erase_block_size = 0,          //erase not necessary
    .write_block_size = UINT32_MAX  //blocks don't have a limit to write at once
};


static void init(blockdevice_t* bd) { // TODO SPI as param
  (void)bd; // suppress unused warning
}

static error_t read(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size) {
  blockdevice_stm32_eeprom_t* bd_eeprom = (blockdevice_stm32_eeprom_t*)bd;

  if(size == 0) return SUCCESS;
  if(size > bd_eeprom->size) return -ESIZE;

  addr += DATA_EEPROM_BASE + bd_eeprom->offset;
  if(addr + size > DATA_EEPROM_BANK2_END) return -ESIZE;

  memcpy(data, (const void*)(intptr_t)(addr), size);

  return SUCCESS;
}

static error_t program(blockdevice_t* bd, const uint8_t* data, uint32_t addr, uint32_t size) {
  blockdevice_stm32_eeprom_t* bd_eeprom = (blockdevice_stm32_eeprom_t*)bd;

  if(size == 0) return SUCCESS;
  if(size > bd_eeprom->size) return -ESIZE;

  addr += DATA_EEPROM_BASE + bd_eeprom->offset;
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
