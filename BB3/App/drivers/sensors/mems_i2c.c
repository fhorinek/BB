/*
 * i2c.c
 *
 *  Created on: Sep 17, 2020
 *      Author: horinek
 *
 *      This is wrapper for the HAL i2c IT
 *      After calling IT transfer the source task will suspend until IT is done
 */

#define DEBUG_LEVEL DEBUG_DBG

#include "mems_i2c.h"

volatile mems_i2c_cb_t mems_i2c_cb = NULL;

void mems_i2c_continue()
{
    if (mems_i2c_cb == NULL)
    {
        osThreadFlagsSet(thread_mems, 0x01);
    }
    else
    {
        mems_i2c_cb_t run = mems_i2c_cb;
        mems_i2c_cb = NULL;
        run();
    }
}

bool mems_i2c_test_device(uint8_t addr)
{
    mems_i2c_wait();
    return HAL_I2C_IsDeviceReady(mems_i2c, addr, 1, 100) == HAL_OK;
}


void mems_i2c_wait()
{
	uint32_t start = HAL_GetTick();
    while(HAL_I2C_GetState(mems_i2c) != HAL_I2C_STATE_READY)
    {
    	if (start + 10 < HAL_GetTick())
    	{
    		ERR("Mems I2C stalled, trying to reset");
    		HAL_I2C_Init(mems_i2c);
    		mems_i2c_continue();
    		return;
    	}

    }
}

void mems_i2c_cmd8(uint8_t adr, uint8_t cmd)
{
	HAL_I2C_Master_Transmit_IT(mems_i2c, adr, &cmd, 1);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);
}

void mems_i2c_cmd8_start(uint8_t adr, uint8_t cmd, mems_i2c_cb_t cb)
{
    static uint8_t scmd = 0;
    scmd = cmd;
    mems_i2c_cb = cb;
    HAL_I2C_Master_Transmit_IT(mems_i2c, adr, &scmd, 1);
}

uint8_t mems_i2c_read8(uint8_t adr, uint8_t reg)
{
	uint8_t tmp;

	HAL_I2C_Mem_Read_IT(mems_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, &tmp, 1);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);

	return tmp;
}

void mems_i2c_write8(uint8_t adr, uint8_t reg, uint8_t val)
{
    HAL_I2C_Mem_Write_IT(mems_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, &val, 1);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);
}

uint16_t mems_i2c_read16(uint8_t adr, uint8_t reg)
{
	uint16_t tmp;

	HAL_I2C_Mem_Read_IT(mems_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&tmp, 2);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);

	return tmp;
}

void mems_i2c_read16_start(uint8_t adr, uint8_t reg, uint16_t * buff, mems_i2c_cb_t cb)
{
    mems_i2c_cb = cb;
    HAL_I2C_Mem_Read_IT(mems_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&buff, 2);
}

void mems_i2c_write16(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg, uint16_t val)
{
    HAL_I2C_Mem_Write_IT(mems_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&val, 2);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);
}

uint32_t mems_i2c_read24(I2C_HandleTypeDef * hi2c, uint8_t adr, uint8_t reg)
{
	uint32_t tmp;

	HAL_I2C_Mem_Read_IT(mems_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&tmp, 3);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);

	return tmp;
}

void mems_i2c_read24_start(uint8_t adr, uint8_t reg, uint32_t * buff, mems_i2c_cb_t cb)
{
    mems_i2c_cb = cb;
    HAL_I2C_Mem_Read_IT(mems_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)buff, 3);
}

void mems_i2c_read_block_start(uint8_t adr, uint8_t reg, uint8_t * buff, uint8_t len, mems_i2c_cb_t cb)
{
    mems_i2c_cb = cb;
    HAL_I2C_Mem_Read_IT(mems_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, buff, len);
}

