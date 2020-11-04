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
#include "pwr_mng.h"

#define BUTTON_PRESSED(A)  (HAL_GPIO_ReadPin(A) == LOW)
#define BUTTON_WAIT(A)      while(BUTTON_PRESSED(BT2))

bool button_hold(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    uint16_t cnt = 0;
    while (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == LOW)
    {
        cnt++;
        HAL_Delay(10);
        if (cnt > 30)
            return true;
    }

    return false;
}

void app_deinit()
{
	//deinit
	INFO("Bootloader power off");
	HAL_Delay(2);

	//MX_GPIO_Init();

	//MX_DMA_Init();
	HAL_DMA_DeInit(&tft_dma);

	//void MX_MDMA_Init();
	HAL_MDMA_DeInit(&hmdma_mdma_channel40_sdmmc1_command_end_0);

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

//    HAL_RCC_DeInit();
//	HAL_DeInit();
}

#define POWER_ON_USB    0
#define POWER_ON_BUTTON 1
#define POWER_ON_TORCH  2

uint8_t app_poweroff()
{
    MX_GPIO_Init();
    MX_TIM2_Init();

    while (1)
    {
        //usb connected
        if (HAL_GPIO_ReadPin(USB_DATA_DET) == HIGH)
            return POWER_ON_USB;

        if (HAL_GPIO_ReadPin(PWR_INT) == LOW)
            return POWER_ON_USB;

        if (button_hold(BT2))
            return POWER_ON_TORCH;

        if (button_hold(BT3))
            return POWER_ON_BUTTON;

        //HAL_PWREx_EnterSTOP2Mode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    }
}

void app_reset()
{
    gfx_clear();

    //prevent to go to factory OTG bootloader
    BUTTON_WAIT(BT2);

    NVIC_SystemReset();
}

void app_main(uint8_t power_on_mode)
{
	bool updated = false;
	bool skip_crc = false;

	INFO("Bootloader init");

    //main power on
    GpioWrite(VCC_MAIN_EN, HIGH);

    //pwm init
    HAL_TIM_Base_Start(&led_timmer);
    HAL_TIM_PWM_Start(&led_timmer, led_bclk);
    HAL_TIM_PWM_Start(&led_timmer, led_torch);

    HAL_Delay(1000);

    pwr_init();


	if (power_on_mode == POWER_ON_USB)
	{
	    //if usb mode exited with usb disconnect, power off
	    if (!msc_loop())
	    {
	        app_reset();
	    }
	}

    if (power_on_mode == POWER_ON_TORCH)
    {
        gui_set_backlight(10);
        gui_set_torch(10);

        gfx_draw_status(GFX_STATUS_TORCH, NULL);

        BUTTON_WAIT(BT2);
        BUTTON_WAIT(BT3);
        BUTTON_WAIT(BT4);

        while(1)
        {
            if (button_hold(BT2))
                break;

            if (button_hold(BT3))
                break;

            if (button_hold(BT4))
                break;
        }

        app_reset();
    }

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
    }

    app_reset();

    //not possible to reach!
    while(1);
}
