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
#include "drivers/tft/tft.h"
#include "drivers/power/led.h"
#include "drivers/esp/esp.h"
#include "drivers/gnss/gnss_ublox_m8.h"
#include "drivers/gnss/fanet.h"

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

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == esp_uart)
	{
		esp_uart_rx_irq_cb();
	}

	if (huart == gnss_uart)
	{
		gnss_uart_rx_irq_ht();
	}

	if (huart == fanet_uart)
	{
		gnss_uart_rx_irq_ht();
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == esp_uart)
	{
		esp_uart_rx_irq_cb();
	}

	if (huart == gnss_uart)
	{
		gnss_uart_rx_irq_tc();
	}

	if (huart == fanet_uart)
	{
		gnss_uart_rx_irq_tc();
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart == gnss_uart)
	{
		//Frame error can be triggered by baudrate change
		ublox_start_dma();
	}
	else if (huart == fanet_uart)
	{
		//Frame error can be triggered by baudrate change
		fanet_start_dma();
	}
	else if (huart == esp_uart)
	{
		//Frame error can be triggered by baudrate change
		esp_start_dma();
	}
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
