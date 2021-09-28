/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* \file
 *
 * Glue functions needed to run STMicroelectronic's LSM303AGR_ACC driver on top of our HAL
 *
 * @author glenn.ergeerts@uantwerpen.be
 */

#include "types.h"
#include "hwi2c.h"
#include "LSM303AGR_ACC_driver.h"

/**
 * @brief  Writes a buffer to the sensor
 * @param  handle instance handle
 * @param  WriteAddr specifies the internal sensor address register to be written to
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToWrite number of bytes to be written
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */

uint8_t LSM303AGR_IO_Write(void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite) {
  if (nBytesToWrite > 1)
    WriteAddr |= 0x80; // Enable I2C multi-bytes write

  if(i2c_write_memory(handle, LSM303AGR_ACC_I2C_ADDRESS, WriteAddr, 8, pBuffer, nBytesToWrite))
    return 0;
  else
    return 1;
}

/**
 * @brief  Reads a from the sensor to buffer
 * @param  handle instance handle
 * @param  ReadAddr specifies the internal sensor address register to be read from
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToRead number of bytes to be read
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t LSM303AGR_IO_Read(void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead) {
  if (nBytesToRead > 1)
    ReadAddr |= 0x80; // Enable I2C multi-bytes read

  if(i2c_read_memory(handle, LSM303AGR_ACC_I2C_ADDRESS, ReadAddr, 8, pBuffer, nBytesToRead))
    return 0;
  else
    return 1;
}
