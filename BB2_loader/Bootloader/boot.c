/*
 * boot.cc
 *
 *  Created on: Jun 10, 2020
 *      Author: horinek
 */

#include "common.h"

#include "drivers/sd.h"
#include "lib/stm32-bootloader/bootloader.h"

#include "gfx.h"
#include "msc.h"
#include "flash.h"

void app_deinit()
{
	//deinit
	INFO("Bootloader deinit");
	HAL_Delay(2);

	//MX_GPIO_Init();

	//MX_DMA_Init();
	HAL_DMA_DeInit(&tft_dma);

	//void MX_MDMA_Init();
	HAL_MDMA_DeInit(&hmdma_mdma_channel40_sdmmc1_end_data_0);

	//MX_FMC_Init();
	HAL_SRAM_DeInit(&hsram1);

	//MX_FATFS_Init();
	FATFS_UnLinkDriver(SDPath);

	//MX_SDMMC1_SD_Init();
	HAL_SD_DeInit(&hsd1);
	sd_unmount();

	//MX_TIM2_Init();
	HAL_TIM_PWM_DeInit(&htim2);
	HAL_TIM_Base_DeInit(&htim2);

	//MX_UART7_Init();
	HAL_UART_DeInit(&huart7);

	//MX_USB_DEVICE_Init();
	//in usb loop

	//MX_CRC_Init();
	HAL_CRC_DeInit(&hcrc);

	//main power off
	GpioSetDirection(VCC_MAIN_EN, INPUT, GPIO_NOPULL);

    HAL_RCC_DeInit();
	HAL_DeInit();
}

void app_main()
{
	bool updated;
	bool usb_connected;
	bool skip_crc = false;

	INFO("Bootloader init");

	//main power on
	GpioSetDirection(VCC_MAIN_EN, OUTPUT, GPIO_NOPULL);
	GpioWrite(VCC_MAIN_EN, LOW);
	HAL_Delay(100);
	GpioWrite(VCC_MAIN_EN, HIGH);

	//power up the negotiator
	GpioSetDirection(CH_EN_OTG, OUTPUT, GPIO_NOPULL);
	GpioWrite(CH_EN_OTG, HIGH);
	HAL_Delay(100);
	GpioSetDirection(CH_EN_OTG, INPUT, GPIO_NOPULL);

	while(1)
	{

//		if (!sd_detect())
//		{
//			gfx_draw_status(GFX_STATUS_ERROR, "No SD card");
//			HAL_Delay(MSG_DELAY);
//
//			while(!sd_detect());
//		}


		usb_connected = msc_loop();

		if (sd_mount())
		{
			FILINFO fno;

			updated = flash_loop();

			if (f_stat(SKIP_CRC_FILE, &fno) == FR_OK)
			{
				WARN("CRC check override!");
				skip_crc = true;
			}
		}
		else
		{
			gfx_draw_status(GFX_STATUS_ERROR, "SD card error");
			HAL_Delay(MSG_DELAY);
		}

		if (flash_verify() || skip_crc)
		{
			if (updated)
			{
				gfx_draw_status(GFX_STATUS_SUCCESS, "Firmware updated");
				HAL_Delay(MSG_DELAY);
			}

			app_deinit();

			Bootloader_JumpToApplication();
		}
		else
		{
			gfx_draw_status(GFX_STATUS_ERROR, "Firmware not valid");
			HAL_Delay(MSG_DELAY);

			if (!usb_connected)
			{
				app_deinit();
				//TODO: poweroff

				while(1);
			}
		}
	}
}
