/*
 * i2c.h
 *
 *  Created on: 4. 12. 2020
 *      Author: horinek
 */

#ifndef MAIN_DRIVERS_I2C_H_
#define MAIN_DRIVERS_I2C_H_

#include "../common.h"

esp_err_t i2c_init(void);
uint8_t i2c_read(uint8_t addr, uint8_t reg);
void i2c_write(uint8_t addr, uint8_t reg, uint8_t val);

#endif /* MAIN_DRIVERS_I2C_H_ */
