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
#include "drivers/esp/protocol.h"
#include "drivers/gnss/gnss_thread.h"
#include "drivers/gnss/gnss_ublox_m8.h"
#include "drivers/gnss/fanet.h"
#include "gui/gui_thread.h"
#include "system/debug_thread.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
        case DISP_TE_Pin:
            tft_irq_display_te();
        break;

        case BT1_Pin:
            gui_set_pin(0);
        break;

//        case BT2_Pin:
//            gui_set_pin(1);
//        break;

        case BT3_Pin:
            gui_set_pin(2);
        break;

        case BT4_Pin:
            gui_set_pin(3);
        break;

        case BT5_Pin:
            gui_set_pin(4);
        break;
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == debug_uart)
	{
		debug_uart_done();
	}

    if (huart == fanet_uart)
    {
        fanet_tx_done();
    }

    if (huart == esp_uart)
    {
        su_tx_done(&protocol_tx);
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
		ublox_start_dma();
	}
	else if (huart == fanet_uart)
	{
		fanet_start_dma();
	}
	else if (huart == esp_uart)
	{
		esp_start_dma();
	}
    else if (huart == debug_uart)
    {
        debug_uart_error();
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
			default:
            break;
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

