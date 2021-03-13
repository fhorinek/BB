/*
 * i2c.h
 *
 *  Created on: Sep 17, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_LL_MEMS_I2C_H_
#define DRIVERS_LL_MEMS_I2C_H_

#include "common.h"

typedef void (* mems_i2c_cb_t)();

void mems_i2c_continue();
void mems_i2c_wait();

bool mems_i2c_test_device(uint8_t addr);

void mems_i2c_cmd8(uint8_t adr, uint8_t cmd);
void mems_i2c_cmd8_start(uint8_t adr, uint8_t cmd, mems_i2c_cb_t cb);

uint8_t mems_i2c_read8(uint8_t adr, uint8_t reg);
void mems_i2c_write8(uint8_t adr, uint8_t reg, uint8_t val);

uint16_t mems_i2c_read16(uint8_t adr, uint8_t reg);
void mems_i2c_write16(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg, uint16_t val);
void mems_i2c_read16_start(uint8_t adr, uint8_t reg, uint16_t * buff, mems_i2c_cb_t cb);

uint32_t mems_i2c_read24(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg);
void mems_i2c_read24_start(uint8_t adr, uint8_t reg, uint32_t * buff, mems_i2c_cb_t cb);

void mems_i2c_read_block_start(uint8_t adr, uint8_t reg, uint8_t * buff, uint8_t len, mems_i2c_cb_t cb);

#endif /* DRIVERS_LL_MEMS_I2C_H_ */
