/*
 * boot.cc
 *
 *  Created on: Jun 10, 2020
 *      Author: horinek
 */

#include "common.h"

#include "drivers/sd.h"
#include "lib/stm32-bootloader/bootloader.h"

#include "nvm.h"

#include "gfx.h"
#include "msc.h"
#include "flash.h"
#include "pwr_mng.h"

#include "drivers/bq25895.h"
#include "drivers/led.h"

void app_deinit()
{
	//deinit
	INFO("Bootloader deinit");
	HAL_Delay(2);

	MX_GPIO_Deinit();

	//MX_DMA_Init();
	HAL_DMA_DeInit(tft_dma);

	//MX_SDMMC1_SD_Init();
	sd_unmount();
	HAL_SD_DeInit(&hsd1);

	//void MX_MDMA_Init();
	HAL_MDMA_DeInit(&hmdma_mdma_channel40_sdmmc1_end_data_0);

	//MX_FMC_Init();
	HAL_SRAM_DeInit(&hsram1);

	//MX_FATFS_Init();
	FATFS_UnLinkDriver(SDPath);

	//MX_TIM2_Init();
    HAL_TIM_PWM_DeInit(disp_timer);
    HAL_TIM_Base_DeInit(disp_timer);
    HAL_TIM_PWM_DeInit(led_timer);
    HAL_TIM_Base_DeInit(led_timer);

	//MX_UART7_Init();
    HAL_UART_DeInit(&huart4);
    HAL_UART_DeInit(&huart7);

	//MX_USB_DEVICE_Init();
	//in usb loop

	//MX_CRC_Init();
	HAL_CRC_DeInit(&hcrc);

	//main power off
	GpioSetDirection(VCC_MAIN_EN, INPUT, GPIO_NOPULL);

	HAL_SuspendTick();
}

#define POWER_ON_USB    0
#define POWER_ON_BUTTON 1
#define POWER_ON_TORCH  2
#define POWER_ON_BOOST  3
#define POWER_ON_REBOOT 4

uint8_t app_poweroff()
{
	if (no_init_check())
    {
        uint8_t boot_type = no_init->boot_type;
        no_init->boot_type = BOOT_NORMAL;
        no_init_update();

        if (boot_type == BOOT_REBOOT)
            return POWER_ON_REBOOT;
    }

    //main power on
    GpioWrite(VCC_MAIN_EN, HIGH);

    HAL_Delay(100);

    bq25895_init();
//    bq25895_batfet_off();
    bq25895_step();

    if (pwr.charge_port > PWR_CHARGE_NONE)
        return POWER_ON_USB;

    bool bq_irq = HAL_GPIO_ReadPin(BQ_INT) == LOW;

    while (1)
    {
        //usb connected
        if (HAL_GPIO_ReadPin(USB_VBUS) == HIGH)
            return POWER_ON_USB;

        if (bq_irq)
            return POWER_ON_USB;

        if (button_hold_2(BT1, 150))
            return POWER_ON_TORCH;

        if (button_hold_2(BT3, 150))
            return POWER_ON_BUTTON;

        if (button_hold_2(BT4, 150))
            return POWER_ON_BOOST;

        //do not sleep if button is pressed
        if (button_pressed(BT1) || button_pressed(BT3) || button_pressed(BT4))
            continue;

        //main power off, backlight off
        GpioWrite(VCC_MAIN_EN, LOW);
        GpioWrite(DISP_BCKL, LOW);

        HAL_PWREx_EnterSTOP2Mode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

        //check right after wake up INR is only 256us long
        bq_irq = HAL_GPIO_ReadPin(BQ_INT) == LOW;

        //restore system clock
        SystemClock_Config();
//        PeriphCommonClock_Config();
    }
}

void app_reset()
{
    gfx_clear();

    //prevent to go to factory OTG bootloader
    button_wait(BT2);

    NVIC_SystemReset();
}

void app_sleep()
{
    led_set_backlight(0);
    led_set_torch(0);

    HAL_Delay(1000);

    app_reset();
}

void torch_loop()
{
    gfx_draw_status(GFX_STATUS_TORCH, NULL);

    led_set_torch(100);

    //wait for buttons to be released
    button_wait(BT1);
    button_wait(BT2);
    button_wait(BT3);

    //if button 1 - 3 pressed stop the flashlight
    while(1)
    {
        if (button_hold(BT1))
            break;

        if (button_hold(BT2))
            break;

        if (button_hold(BT3))
            break;
    }
}

void app_main(uint8_t power_on_mode)
{
	bool updated = false;
	bool skip_crc = false;

	INFO("Bootloader init");

    //main power on
    GpioWrite(VCC_MAIN_EN, HIGH);

    //pwm init
    led_init();

    HAL_Delay(100);

    pwr_init();

    if (power_on_mode == POWER_ON_TORCH)
    {
    	torch_loop();
        app_sleep();
    }

    if (power_on_mode == POWER_ON_BOOST)
    {
        pwr_data_mode(dm_host_boost);
        power_on_mode = POWER_ON_USB;
    }

    if (sd_mount())
    {
    	if (file_exists(DEV_MODE_FILE))
    		development_mode = true;

    	bool format = file_exists(FORMAT_FILE);

    	sd_unmount();

    	if (format)
    	{
    		sd_format();
    		sd_set_disk_label();
    	}
    }

	if (power_on_mode == POWER_ON_USB)
	{
	    //if usb mode exited with usb disconnect, power off
	    if (!msc_loop())
	    {
	        app_sleep();
	    }
	}

    if (sd_mount())
    {
        updated = flash_loop();

        if (file_exists(SKIP_CRC_FILE))
        {
            WARN("CRC check override!");
            skip_crc = true;
        }
    }
    else
    {
        gfx_draw_status(GFX_STATUS_ERROR, "SD card error");
        button_confirm(BT3);
    }

    if (flash_verify() || skip_crc)
    {
        if (updated)
        {
            gfx_draw_status(GFX_STATUS_SUCCESS, "Firmware updated");
            button_confirm(BT3);
        }

        if (*(uint32_t *)APP_ADDRESS == 0xFFFFFFFF)
        {
            gfx_draw_status(GFX_STATUS_ERROR, "No Firmware");
            button_confirm(BT3);
        }
        else
        {
			app_deinit();
			Bootloader_JumpToApplication();
        }
    }
    else
    {
        gfx_draw_status(GFX_STATUS_ERROR, "Firmware not valid");
        button_confirm(BT3);
    }

    app_reset();

    //not possible to reach!
    while(1);
}
