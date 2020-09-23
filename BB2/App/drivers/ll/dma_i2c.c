/*
 * dma_i2c.c
 *
 *  Created on: Sep 17, 2020
 *      Author: horinek
 *
 *      This is wrapper for the HAL i2c dma
 *      After calling dma transfer the source task will suspend until dma is done
 */

#include "dma_i2c.h"

void dma_i2c_continue(I2C_HandleTypeDef *hi2c)
{
	if (hi2c == &mems_i2c)
		vTaskNotifyGiveFromISR((TaskHandle_t)MEMSHandle, NULL);
	else if (hi2c == &sys_i2c)
		vTaskNotifyGiveFromISR((TaskHandle_t)SystemHandle, NULL);
}

void dma_i2c_cmd8(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t cmd)
{
	HAL_I2C_Master_Transmit_DMA(hi2c, adr, &cmd, 1);
	ulTaskNotifyTake(true, WAIT_INF);
}


uint8_t dma_i2c_read8(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg)
{
	uint8_t tmp;

	HAL_I2C_Mem_Read_DMA(hi2c, adr, reg, I2C_MEMADD_SIZE_8BIT, &tmp, 1);
	ulTaskNotifyTake(true, WAIT_INF);

	return tmp;
}

uint16_t dma_i2c_read16(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg)
{
	uint16_t tmp;

	HAL_I2C_Mem_Read_DMA(hi2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&tmp, 2);
	ulTaskNotifyTake(true, WAIT_INF);

	return tmp;
}

uint32_t dma_i2c_read24(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg)
{
	uint32_t tmp;

	HAL_I2C_Mem_Read_DMA(hi2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&tmp, 3);
	ulTaskNotifyTake(true, WAIT_INF);

	return tmp;
}

void dma_i2c_read_block(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg, void * data, uint8_t len)
{
	HAL_I2C_Mem_Read_DMA(hi2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *) data, len);
	ulTaskNotifyTake(true, WAIT_INF);
}
