
#define DEBUG_LEVEL DBG_DEBUG

#include "system_i2c.h"

#define SYSTEM_I2C_WAIT_TIME 10

void system_i2c_continue()
{
    osThreadFlagsSet(thread_system, 0x01);
}

bool system_i2c_wait()
{
	static uint32_t recover_cnt = 0;

	uint32_t retflag = osThreadFlagsWait(0x01, osFlagsWaitAny, SYSTEM_I2C_WAIT_TIME);
	if (retflag == osErrorTimeout)
	{
		WARN("Timeout, trying to recover system i2c (%lu)", recover_cnt++);
		system_i2c_continue();
		HAL_I2C_Init(sys_i2c);

		return false;
	}

	return true;
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
	if (!system_i2c_wait())
	{
		system_i2c_cmd8(adr, cmd);
	}

}

uint8_t system_i2c_read8(uint8_t adr, uint8_t reg)
{
	uint8_t tmp;

	HAL_I2C_Mem_Read_IT(sys_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, &tmp, 1);
	if (!system_i2c_wait())
	{
		return system_i2c_read8(adr, reg);
	}

	return tmp;
}

void system_i2c_write8(uint8_t adr, uint8_t reg, uint8_t val)
{
    HAL_I2C_Mem_Write_IT(sys_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, &val, 1);
	if (!system_i2c_wait())
	{
		system_i2c_write8(adr, reg, val);
	}

}

uint16_t system_i2c_read16(uint8_t adr, uint8_t reg)
{
	uint16_t tmp;

	HAL_I2C_Mem_Read_IT(sys_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&tmp, 2);
	if (!system_i2c_wait())
	{
		return system_i2c_read16(adr, reg);
	}

	return tmp;
}

void system_i2c_write16(uint8_t adr, uint8_t reg, uint16_t val)
{
    HAL_I2C_Mem_Write_IT(sys_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&val, 2);
	if (!system_i2c_wait())
	{
		system_i2c_write16(adr, reg, val);
	}
}

uint32_t system_i2c_read24(uint8_t adr, uint8_t reg)
{
	uint32_t tmp;

	HAL_I2C_Mem_Read_IT(sys_i2c, adr, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&tmp, 3);
	if (!system_i2c_wait())
	{
		return system_i2c_read24(adr, reg);
	}

	return tmp;
}

