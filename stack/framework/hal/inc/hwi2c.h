/*
 * hwi2c.h
 *
 *  Created on: Aug 25, 2015
 *      Author: peryton
 */

#ifndef I2C_H_
#define I2C_H_

#include "types.h"
#include "link_c.h"


typedef struct {
  uint8_t com_code;
  uint8_t byte_cnt;
  uint8_t data[];
}i2c_message_typeDef;

__LINK_C void i2c_init();
__LINK_C void i2c_write(uint8_t address,uint8_t* tx_buffer,int length);
__LINK_C void i2c_read(uint8_t address,uint8_t* rx_buffer, int length);
__LINK_C void i2c_write_read(uint8_t address,uint8_t* tx_buffer,int lengthtx,uint8_t* rx_buffer,int lengthrx);

/* I2C functions */
__LINK_C void print_byte_setup(uint8_t byte);




#endif /* FRAMEWORK_HAL_INC_HWI2C_H_ */
