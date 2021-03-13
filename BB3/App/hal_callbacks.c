/*
 * hal_callbacks.c
 *
 *  Created on: Sep 17, 2020
 *      Author: horinek
 |
 *      This file is router for HAL callbacks
 */


#include "common.h"

#include "drivers/sensors/mems_i2c.h"
#include "drivers/power/system_i2c.h"
#include "drivers/tft_hx8352.h"
#include "drivers/power/led.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == DISP_TE_Pin)
	{
		tft_irq_display_te();
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == debug_uart)
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
    if (hi2c == mems_i2c)
        mems_i2c_continue();
    else
        system_i2c_continue();
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == mems_i2c)
        mems_i2c_continue();
    else
        system_i2c_continue();
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == mems_i2c)
        mems_i2c_continue();
    else
        system_i2c_continue();
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == mems_i2c)
        mems_i2c_continue();
    else
        system_i2c_continue();
}

void mems_phase1();
void mems_phase2();
void mems_phase3();
void mems_phase4();

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == meas_timer)
	{
		switch (htim->Channel)
		{
			case(HAL_TIM_ACTIVE_CHANNEL_1):
				mems_phase1();
			break;
			case(HAL_TIM_ACTIVE_CHANNEL_2):
				mems_phase2();
			break;
//			case(HAL_TIM_ACTIVE_CHANNEL_3):
//				mems_phase3();
//			break;
//			case(HAL_TIM_ACTIVE_CHANNEL_4):
//				mems_phase4();
//			break;
		}

	}
}

void HAL_TIM_PeriodElapsedCallback2(TIM_HandleTypeDef *htim)
{
    if (htim == rtos_timer)
    {
        rtos_timer_elapsed();
    }

    if (htim == disp_timer)
    {
        disp_period_irq();
    }

    if (htim == led_timer)
    {
        led_period_irq();
    }
}


void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    spi_dma_done_cb();
}
