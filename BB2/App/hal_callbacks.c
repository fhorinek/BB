/*
 * hal_callbacks.c
 *
 *  Created on: Sep 17, 2020
 *      Author: horinek
 |
 *      This file is router for HAL callbacks
 */


#include "common.h"

#include "drivers/ll/dma_i2c.h"
#include "drivers/tft_hx8352.h"
#include "drivers/led.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == DISP_TE_Pin)
	{
		tft_irq_display_te();
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &debug_uart)
	{
		debug_uart_done();
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
//	while(1);
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	dma_i2c_continue(hi2c);
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	dma_i2c_continue(hi2c);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	dma_i2c_continue(hi2c);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	dma_i2c_continue(hi2c);
}

void mems_meas_phase1();
void mems_meas_phase2();
void mems_meas_phase3();
void mems_meas_phase4();

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &meas_timer)
	{
		switch (htim->Channel)
		{
			case(HAL_TIM_ACTIVE_CHANNEL_1):
				mems_meas_phase1();
			break;
			case(HAL_TIM_ACTIVE_CHANNEL_2):
				mems_meas_phase2();
			break;
			case(HAL_TIM_ACTIVE_CHANNEL_3):
				mems_meas_phase3();
			break;
			case(HAL_TIM_ACTIVE_CHANNEL_4):
				mems_meas_phase4();
			break;
		}

	}
}

void HAL_TIM_PeriodElapsedCallback2(TIM_HandleTypeDef *htim)
{
	if (htim == &led_timmer)
	{
		led_period_irq();
	}
}

