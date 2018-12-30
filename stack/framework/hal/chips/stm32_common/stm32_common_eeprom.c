
#include "stm32_common_eeprom.h"
#include "stm32_device.h"
#include "debug.h"
#include "log.h"
#include "string.h"

// TODO validate embedded EEPROM works the same on stm32l1 and uses same start address and size as well

#define DPRINT(...) log_print_string(__VA_ARGS__)
#define DPRINT_DATA(p, n) log_print_data(p, n)

// forward declare driver function pointers
static void init(blockdevice_t* bd);
static error_t read(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size);
static error_t program(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size);

blockdevice_driver_t blockdevice_driver_stm32_eeprom = {
    .init = init,
    .read = read,
    .program = program,
};


static void init(blockdevice_t* bd) { // TODO SPI as param
  DPRINT("init EEPROM block device\n");
  (void)bd; // suppress unused warning
}

static error_t read(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size) {
  (void)bd; // suppress unused warning
  DPRINT("BD READ %i @ %x\n", size, addr);

  if(size == 0) return SUCCESS;

  addr += DATA_EEPROM_BASE;
  if(addr + size > DATA_EEPROM_BANK2_END) return -ESIZE;

  memcpy(data, (const void*)(addr), size);

  return SUCCESS;
}

static error_t program(blockdevice_t* bd, uint8_t* data, uint32_t addr, uint32_t size) {
  (void)bd; // suppress unused warning
  DPRINT("BD WRITE %i @ %x\n", size, addr);

  if(size == 0) return SUCCESS;

  addr += DATA_EEPROM_BASE;
  if(addr + size > DATA_EEPROM_BANK2_END) return -ESIZE;

  HAL_FLASHEx_DATAEEPROM_Unlock();
  if(FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK)
    return -ETIMEDOUT;

  for (size_t i = 0; i < size; i++) {
      *(uint8_t*)(addr + i) = data[i];
  }

  if(FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK)
    return -ETIMEDOUT;

  HAL_FLASHEx_DATAEEPROM_Lock();

  DPRINT_DATA(data, size);

  return SUCCESS;
}
