/*
 * dma_i2c.h
 *
 *  Created on: Sep 17, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_LL_DMA_I2C_H_
#define DRIVERS_LL_DMA_I2C_H_

#include "../../common.h"

void dma_i2c_continue(I2C_HandleTypeDef *hi2c);

void dma_i2c_cmd8(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t cmd);

uint8_t dma_i2c_read8(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg);
uint16_t dma_i2c_read16(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg);
uint32_t dma_i2c_read24(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg);
void dma_i2c_read_block(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg, void * data, uint8_t len);

#endif /* DRIVERS_LL_DMA_I2C_H_ */
