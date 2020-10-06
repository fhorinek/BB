/*
 * mems.c
 *
 *  Created on: Sep 14, 2020
 *      Author: horinek
 */

#include "../common.h"

#include "../drivers/ms5611.h"
#include "../fc/fc.h"

//Mems
//1E - acc + mag
//6B - gyro
//77 - ms8266

//system
//36 - fuel gauge
//6A - charger

void task_MEMS(void *argument)
{
	vTaskSuspend(NULL);
	INFO("MEMS started");

	DBG("Mems i2c");
	for (uint8_t adr = 0; adr <= 128; adr++)
	{
		if (HAL_I2C_IsDeviceReady(&mems_i2c, adr << 1, 1, 100) == HAL_OK)
			DBG(" %02X %u", adr, adr);
	}
	DBG("---");

	DBG("sys i2c");
	for (uint8_t adr = 0; adr <= 128; adr++)
	{
		if (HAL_I2C_IsDeviceReady(&sys_i2c, adr << 1, 1, 100) == HAL_OK)
			DBG(" %02X %u", adr, adr);
	}
	DBG("---");

	GpioSetDirection(REV_VCC, OUTPUT, GPIO_NOPULL);
	HAL_TIM_Base_Start(&meas_timer);
	HAL_TIM_OC_Start_IT(&meas_timer, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(&meas_timer, TIM_CHANNEL_2);
	HAL_TIM_OC_Start_IT(&meas_timer, TIM_CHANNEL_3);
	HAL_TIM_OC_Start_IT(&meas_timer, TIM_CHANNEL_4);

	ms5611_Init();


	vTaskSuspend(NULL);
}

void mems_meas_phase1() 	//t = 0
{
	ms5611_ReadPressure();
	ms5611_StartTemperature();
}

void mems_meas_phase2()		//t = 0.78ms
{
	ms5611_ReadTemperature();
	ms5611_StartPressure();
	ms5611_CompensateTemperature();

	fc.vario.pressure = ms5611_CompensatePressure();
}

void mems_meas_phase3()		//t = 2ms
{
}

void mems_meas_phase4()		//t = 6ms
{
}
