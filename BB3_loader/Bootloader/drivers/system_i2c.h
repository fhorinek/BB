/*
 * i2c.h
 *
 *  Created on: Sep 17, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_LL_SYSTEM_I2C_H_
#define DRIVERS_LL_SYSTEM_I2C_H_

#include "common.h"

bool system_i2c_test_device(uint8_t addr);

void system_i2c_cmd8(uint8_t adr, uint8_t cmd);

uint8_t system_i2c_read8(uint8_t adr, uint8_t reg);
void system_i2c_write8(uint8_t adr, uint8_t reg, uint8_t val);

uint16_t system_i2c_read16(uint8_t adr, uint8_t reg);
void system_i2c_write16(uint8_t adr, uint8_t reg, uint16_t val);

uint32_t system_i2c_read24(uint8_t adr, uint8_t reg);

#endif /* DRIVERS_LL_SYSTEM_I2C_H_ */
