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
//    HAL_TIM_PWM_DeInit(disp_timer);
//    HAL_TIM_Base_DeInit(disp_timer);
    HAL_NVIC_DisableIRQ(TIM2_IRQn);

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

#define POWER_ON_USB    		0
#define POWER_ON_BUTTON			1
#define POWER_ON_TORCH  		2
#define POWER_ON_BOOST  		3
#define POWER_ON_REBOOT 		4

uint8_t app_poweroff()
{
	uint8_t boot_type = BOOT_SHOW;
	if (no_init_check())
    {
        boot_type = no_init->boot_type;
        no_init->boot_type = BOOT_SLEEP;
        no_init_update();

        if (boot_type == BOOT_REBOOT)
            return POWER_ON_REBOOT;
    }
    if (boot_type == BOOT_SHOW)
        return POWER_ON_USB;

    //main power on
    GpioWrite(VCC_MAIN_EN, HIGH);

    HAL_Delay(100);

//    bq25895_init();
//    max17260_init();
//
////    //enable boost for negotiator
////    GpioWrite(BQ_OTG, HIGH);
////    //enable alt charger
////    GpioWrite(ALT_CH_EN, LOW);

    pwr_init();
    GpioWrite(BQ_OTG, LOW);
    bq25895_batfet_off();


    HAL_I2C_DeInit(&hi2c2);

    //charge port connected, but charging is not done
    if (pwr.charge_port > PWR_CHARGE_NONE
    		&& !(pwr.charge_port == PWR_CHARGE_DONE && boot_type == BOOT_CHARGE))
        return POWER_ON_USB;

    bool bq_irq = HAL_GPIO_ReadPin(BQ_INT) == LOW;

    while (1)
    {
        //usb connected, but charging is not done
        if (HAL_GPIO_ReadPin(USB_VBUS) == HIGH
        		&& !(pwr.data_port == PWR_DATA_CHARGE_DONE && boot_type == BOOT_CHARGE))
            return POWER_ON_USB;

        //usb disconnected, but charging was done
        if (HAL_GPIO_ReadPin(USB_VBUS) == LOW
        		&& pwr.data_port == PWR_DATA_CHARGE_DONE
				&& boot_type == BOOT_CHARGE)
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

        //alt ch on, main power off, backlight off
        GpioWrite(ALT_CH_EN, LOW);
        GpioWrite(VCC_MAIN_EN, LOW);
        GpioWrite(DISP_BCKL, LOW);

        HAL_PWREx_EnterSTOP2Mode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

        //check right after wake up IRQ is only 256us long
        bq_irq = HAL_GPIO_ReadPin(BQ_INT) == LOW;

        //restore system clock
        SystemClock_Config();

        //PeriphCommonClock not used in this clock configuration
//        PeriphCommonClock_Config();
    }
}

void app_reset()
{
    gfx_clear();

    //prevent to go to factory OTG bootloader
    button_wait(BT4);

    NVIC_SystemReset();
}

void app_sleep()
{
    //set next boot to sleep, except when the charging is done
    if (!no_init_check())
    {
        no_init->boot_type = BOOT_SLEEP;
        no_init_update();
    }
    else
    {
        if (no_init->boot_type != BOOT_CHARGE)
        {
            no_init->boot_type = BOOT_SLEEP;
            no_init_update();
        }
    }

	led_dim();
    app_reset();
}

void app_sleep_show()
{
    //set next boot to show battery status
    no_init->boot_type = BOOT_SHOW;
    no_init_update();

    led_dim();
    app_reset();
}

#define BUTTON_TIME	50
#define HINT_TIME	1000
#define TIMEOUT		3000

void key_combo(uint8_t status, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	gfx_draw_status(status, "");

	uint32_t timer = HAL_GetTick();
	while(1)
	{
		if (!button_pressed(GPIOx, GPIO_Pin))
		{
			if (timer + BUTTON_TIME > HAL_GetTick())
			    app_sleep_show();
			else
				break;
		}

		if (timer + HINT_TIME < HAL_GetTick())
			gfx_draw_status(status, "Release button");

		if (timer + TIMEOUT < HAL_GetTick())
		    app_sleep_show();
	}

	gfx_draw_status(status | GFX_COLOR_MOD, "");

	timer = HAL_GetTick();
	while(1)
	{
		if (button_pressed(GPIOx, GPIO_Pin))
		{
			if (timer + BUTTON_TIME < HAL_GetTick())
				break;
			else
			    app_sleep_show();
		}

		if (timer + HINT_TIME < HAL_GetTick())
			gfx_draw_status(status | GFX_COLOR_MOD, "Press now");

		if (timer + TIMEOUT < HAL_GetTick())
		    app_sleep_show();
	}

	gfx_draw_status(status | GFX_COLOR_MOD, "");

	timer = HAL_GetTick();
	while(1)
	{
		if (!button_pressed(GPIOx, GPIO_Pin))
		{
			if (timer + BUTTON_TIME < HAL_GetTick())
				break;
			else
			    app_sleep_show();
		}

		if (timer + HINT_TIME < HAL_GetTick())
			gfx_draw_status(status| GFX_COLOR_MOD, "Release button");

		if (timer + TIMEOUT < HAL_GetTick())
		    app_sleep_show();
	}
}

void torch_loop()
{
    gfx_draw_status(GFX_STATUS_TORCH, NULL);

	led_set_backlight(GFX_BACKLIGHT);
	led_set_backlight_timeout(GFX_BACKLIGHT_TIME);

	led_set_torch(100);

    //wait for buttons to be released
    button_wait(BT1);
    button_wait(BT2);
    button_wait(BT3);

    //if button 1 - 3 pressed stop the flashlight
    while(1)
    {
    	pwr_step();
    	bat_check_step();

        if (button_hold(BT1))
            break;

        if (button_hold(BT2))
            break;

        if (button_hold(BT3))
            break;

        if (button_pressed(BT1) ||button_pressed(BT2) || button_pressed(BT3) || button_pressed(BT4) || button_pressed(BT5))
        {
        	led_set_backlight(GFX_BACKLIGHT);
        	led_set_backlight_timeout(GFX_BACKLIGHT_TIME);
        }
    }
}

//emergency sdcard format
#define FORMAT_WAIT_SECONDS	10
void format_loop()
{
	uint32_t start = HAL_GetTick();
	bool tgl = true;

	while (button_pressed(BT2) && button_pressed(BT5))
	{
		uint8_t delta = (HAL_GetTick() - start) / 1000;

		if (delta > FORMAT_WAIT_SECONDS)
		{
            Bootloader_Init();
            gfx_draw_status(GFX_STATUS_UPDATE, "Erasing STM");

            nvm_data_t nvm_temp;
            memcpy(&nvm_temp, (uint8_t *)NVM_ADDR, sizeof(nvm_data_t));

            Bootloader_Erase();

            Bootloader_FlashBegin(NVM_ADDR);

			nvm_temp.app.size = 0;
			nvm_temp.app.crc = 0;
			nvm_temp.app.build_number = 0;

			for (uint32_t i = 0; i < sizeof(nvm_data_t); i += 16)
			{
				Bootloader_FlashNext((uint32_t *)(((uint8_t *)&nvm_temp) + i));
			}

            Bootloader_FlashEnd();

    		sd_format();

    		break;
		}

		tgl = !tgl;

		char msg[64];
		if (delta >= FORMAT_WAIT_SECONDS / 2 && tgl)
		{
			strcpy(msg, "");
		}
		else
		{
 			snprintf(msg, sizeof(msg), "Erase in %us", FORMAT_WAIT_SECONDS - delta);
		}

		gfx_draw_status(GFX_STATUS_WARNING, msg);

		HAL_Delay(200);
	}
}

void bat_check_step()
{
	if (pwr.fuel_gauge.bat_voltage < 310
			&& (pwr.data_port == PWR_DATA_NONE || pwr.data_port == PWR_DATA_PASS)
			&& pwr.charge_port == PWR_CHARGE_NONE)
	{
		gfx_draw_status(GFX_STATUS_LOW_BAT, NULL);
		button_confirm(BT3);
		app_sleep();
	}
}

void app_main(uint8_t power_on_mode)
{
	bool updated = false;
	bool skip_crc = false;

	debug_enable();

	INFO("Bootloader init");

    //main power on
    GpioWrite(VCC_MAIN_EN, HIGH);

    //pwm init
    led_init();

    HAL_Delay(100);

    pwr_init();

    if (power_on_mode != POWER_ON_USB)
    {
    	bat_check_step();
    }

    if (button_pressed(BT2) && button_pressed(BT5))
    {
    	format_loop();
    	app_sleep();
    }

    if (power_on_mode == POWER_ON_TORCH)
    {
    	key_combo(GFX_STARTUP_TORCH, BT1);
    	torch_loop();
        app_sleep();
    }

    if (power_on_mode == POWER_ON_BOOST)
    {
    	key_combo(GFX_STARTUP_BAT, BT4);
        pwr_data_mode(dm_host_boost);
        power_on_mode = POWER_ON_USB;
    }

    if (power_on_mode == POWER_ON_BUTTON)
    {
    	key_combo(GFX_STARTUP_APP, BT3);
    }

    //check for FORMAT file
    if (sd_mount())
    {
    	if (file_exists(DEV_MODE_FILE))
    		development_mode = true;

    	bool format = file_exists(FORMAT_FILE);

    	sd_unmount();

    	if (format)
    	{
    		sd_format();
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

		    no_init_check();
		    no_init->boot_type = BOOT_SHOW;
		    no_init_update();

			Bootloader_JumpToApplication();
        }
    }
    else
    {
        gfx_draw_status(GFX_STATUS_ERROR, "Firmware not found");
        button_confirm(BT3);
    }

    app_reset();

    //not possible to reach!
    while(1);
}
