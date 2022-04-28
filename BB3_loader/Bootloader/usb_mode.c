/*
 * msc.cc
 *
 *  Created on: 18. 6. 2020
 *      Author: horinek
 */


#include <usb_mode.h>
#include "gfx.h"

#include "pwr_mng.h"
#include "nvm.h"
#include "drivers/led.h"

#include "app_mtp_cb.h"
#include "app_threadx.h"

void usb_mode_start()
{
    MX_ThreadX_Init();
}

extern bool tft_no_rtos;

void usb_mode_entry(ULONG id)
{
    INFO("USB mode on");

    tft_no_rtos = false;

    PSRAM_init();
    sd_init();

    if (button_pressed(BT2) && button_pressed(BT5))
    {
        format_loop();
        app_sleep();
    }

    if (sd_mount() == false)
    {
        gfx_draw_status(GFX_STATUS_ERROR, "SD card error");
        button_confirm(BT3);
        app_sleep();
    }

    if (file_exists(DEV_MODE_FILE))
    {
        development_mode = true;
    }

	led_set_backlight_timeout(GFX_BACKLIGHT_TIME);

    uint8_t start_up = false;

    uint8_t old_charge = 0xFF;
    uint8_t old_data = 0xFF;
    pwr_data_mode_t old_data_mode = 0xFF;
    uint32_t next_update = 0;

    uint32_t pwr_delay = HAL_GetTick() + 4000;

    bool session_open = false;

    while (1)
    {
        pwr_step();

        if (button_pressed(BT1) || button_pressed(BT2) || button_pressed(BT3) || button_pressed(BT4) || button_pressed(BT5))
        {
        	led_set_backlight(GFX_BACKLIGHT);
        	led_set_backlight_timeout(GFX_BACKLIGHT_TIME);
        }

        //are class data avalible (usb init ok)
        if (mtp_session_is_open())
        {
            session_open = true;

            if (pwr.data_port == PWR_DATA_CHARGE || pwr.data_port == PWR_DATA_CHARGE_DONE)
            {
                pwr.data_port = PWR_DATA_ACTIVE;
            }

            if (pwr.data_port == PWR_DATA_NONE)
            {
                INFO("Port none, session open");
                mtp_session_close_force();
            }
        }
        else
        {
            session_open = false;

        	if (pwr.data_port == PWR_DATA_ACTIVE)
        	{
                INFO("Port active, session closed");
                pwr.data_port = PWR_DATA_NONE;

                //ejected
                break;
        	}
        }

        bool timer_update = next_update < HAL_GetTick();

        //change gfx status if needed
        if (old_charge != pwr.charge_port
        		|| old_data != pwr.data_port
				|| old_data_mode != pwr.data_usb_mode
				|| timer_update)
        {
        	if (!timer_update)
        	{
				led_set_backlight(GFX_BACKLIGHT);
				led_set_backlight_timeout(GFX_BACKLIGHT_TIME);
        	}


            //to update battery percentage
            next_update = HAL_GetTick() + 1000;

            //add timeout so sudden power disconnect will not trigger weak charger warning
            if (old_charge != pwr.charge_port
            		&& old_charge > PWR_CHARGE_WEAK
            		&& pwr.charge_port == PWR_CHARGE_WEAK)
            	pwr_delay = HAL_GetTick() + 2000;

            //INFO("PWR %u %u %u %d", pwr.fuel_gauge.bat_cap, pwr.fuel_gauge.bat_cap_full, pwr.fuel_gauge.bat_voltage, pwr.fuel_gauge.bat_current);

            old_charge = pwr.charge_port;
            old_data = pwr.data_port;
            old_data_mode = pwr.data_usb_mode;

            if (pwr.data_usb_mode != dm_client)
            {
                if (pwr.charge_port > PWR_CHARGE_NONE)
                    gfx_draw_status(GFX_STATUS_CHARGE_PASS, NULL);
                else
                    gfx_draw_status(GFX_STATUS_NONE_BOOST, NULL);
            }
            else
            {
                //#define GFX_STATUS_CHARGE_NONE  0   //4 _
                //#define GFX_STATUS_CHARGE_DATA  0   //4 0
                //#define GFX_STATUS_NONE_DATA    0   //_ 0
                //#define GFX_STATUS_NONE_CHARGE  0   //0 4
                if (pwr.charge_port > PWR_CHARGE_NONE && pwr.data_port != PWR_DATA_ACTIVE)
                    gfx_draw_status(GFX_STATUS_CHARGE_NONE, NULL);

                if (pwr.charge_port > PWR_CHARGE_NONE && pwr.data_port == PWR_DATA_ACTIVE)
                    gfx_draw_status(GFX_STATUS_CHARGE_DATA, NULL);

                if (pwr.charge_port == PWR_CHARGE_NONE && pwr.data_port == PWR_DATA_ACTIVE)
                    gfx_draw_status(GFX_STATUS_NONE_DATA, NULL);

                if (pwr.charge_port == PWR_CHARGE_NONE && (pwr.data_port == PWR_DATA_CHARGE || pwr.data_port == PWR_DATA_CHARGE_DONE))
                    gfx_draw_status(GFX_STATUS_NONE_CHARGE, NULL);
            }

            //cable is disconnected
            if (pwr.charge_port == PWR_CHARGE_NONE && pwr.data_port == PWR_DATA_NONE)
            {
                gfx_draw_status(GFX_STATUS_NONE_NONE, NULL);
            }
        }

        bool anim_done = gfx_draw_anim();
        bool wait_anim_to_end = false;

        //no cable connected, wait for animation
        if (pwr.charge_port == PWR_CHARGE_NONE && pwr.data_port == PWR_DATA_NONE)
        {
        	wait_anim_to_end = true;
        }

		//charge done by alt charger
		if (pwr.charge_port == PWR_CHARGE_NONE
				&& pwr.data_port == PWR_DATA_CHARGE_DONE
				&& pwr_delay < HAL_GetTick())
		{
			no_init->boot_type = BOOT_CHARGE;
			no_init_update();

			wait_anim_to_end = true;
		}

		//charge done by bq charger
		if (pwr.charge_port == PWR_CHARGE_DONE && !session_open)
		{
			no_init->boot_type = BOOT_CHARGE;
			no_init_update();

			wait_anim_to_end = true;
		}

		//usb charging or active but msc unlocked
		if (button_hold(BT3) && !session_open)
		{
			//start right now!
			start_up = true;
			break;
		}

		//weak charger connected
		if (pwr.charge_port == PWR_CHARGE_WEAK
				&& !session_open
				&& pwr_delay < HAL_GetTick())
		{
			gfx_draw_status(GFX_STATUS_ERROR, "Weak charger!");
			button_confirm(BT3);
			start_up = false;
			break;
		}

    	if (wait_anim_to_end)
        {
        	if (anim_done && pwr_delay < HAL_GetTick())
        	{
        		break;
        	}
        }

        if (pwr_delay < HAL_GetTick() && anim_done)
        {
        	bat_check_step();
        }
    }

    INFO("USB mode off");

	led_set_backlight(GFX_BACKLIGHT);
	led_set_backlight_timeout(0);

    if (start_up)
        app_continue();
    else
        app_sleep();
}

