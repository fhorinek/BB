
#define DEBUG_LEVEL DEBUG_DBG

#include "system_i2c.h"

void system_i2c_continue()
{
    osThreadFlagsSet(thread_system, 0x01);
}

bool system_i2c_test_device(uint8_t addr)
{
    while (HAL_I2C_GetState(sys_i2c) != HAL_I2C_STATE_READY)
    {
        taskYIELD();
    }

    return HAL_I2C_IsDeviceReady(sys_i2c, addr, 1, 100) == HAL_OK;
}

void system_i2c_cmd8(uint8_t adr, uint8_t cmd)
{
	HAL_I2C_Master_Transmit_IT(sys_i2c, adr, &cmd, 1);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);
}

uint8_t system_i2c_read8(uint8_t adr, uint8_t reg)
{
	uint8_t tmp;

	HAL_I2C_Mem_Read_IT(sys_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, &tmp, 1);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);

	return tmp;
}

void system_i2c_write8(uint8_t adr, uint8_t reg, uint8_t val)
{
    HAL_I2C_Mem_Write_IT(sys_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, &val, 1);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);
}

uint16_t system_i2c_read16(uint8_t adr, uint8_t reg)
{
	uint16_t tmp;

	uint8_t ret = HAL_I2C_Mem_Read_IT(sys_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&tmp, 2);
	if (ret != HAL_OK)
	    osDelay(1);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);

	return tmp;
}

void system_i2c_write16(uint8_t adr, uint8_t reg, uint16_t val)
{
    HAL_I2C_Mem_Write_IT(sys_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&val, 2);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);
}

uint32_t system_i2c_read24(uint8_t adr, uint8_t reg)
{
	uint32_t tmp;

	HAL_I2C_Mem_Read_IT(sys_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&tmp, 3);
    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);

	return tmp;
}

