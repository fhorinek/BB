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

#include "../../Core/Inc/crc.h"
#include "../../Core/Inc/dma.h"
#include "../../Core/Inc/sdmmc.h"
#include "../../Core/Inc/fmc.h"

extern "C" void app_main();

void app_deinit()
{
	//deinit
	INFO("Bootloader deinit");
	HAL_Delay(2);

	//MX_GPIO_Init();

	//MX_DMA_Init();
	HAL_DMA_DeInit(&hdma_memtomem_dma2_stream0);

	//MX_FMC_Init();
	HAL_SRAM_DeInit(&hsram1);

	//MX_TIM2_Init();
	HAL_TIM_PWM_DeInit(&htim2);
	HAL_TIM_Base_DeInit(&htim2);

	//MX_USART1_UART_Init();
	HAL_UART_DeInit(&huart1);

	//MX_SDMMC1_SD_Init();
	HAL_SD_DeInit(&hsd1);
	sd_unmount();
	sd_deinit();

	//MX_CRC_Init();
	HAL_CRC_DeInit(&hcrc);

    HAL_RCC_DeInit();
	HAL_DeInit();
}

void app_main()
{
	bool updated;
	bool usb_connected;

	INFO("Bootloader init");

	while(1)
	{

		if (!sd_detect())
		{
			gfx_draw_status(GFX_STATUS_ERROR, "No SD card");
			HAL_Delay(MSG_DELAY);

			while(!sd_detect());
		}

		sd_init();

		usb_connected = msc_loop();

		if (sd_mount())
		{
			updated = flash_loop();
		}
		else
		{
			gfx_draw_status(GFX_STATUS_ERROR, "SD card error");
			HAL_Delay(MSG_DELAY);
		}

		if (flash_verify())
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
