/*
 * msc.cc
 *
 *  Created on: 18. 6. 2020
 *      Author: horinek
 */


#include "msc.h"

#include "gfx.h"

#include "pwr_mng.h"
#include "nvm.h"
#include "drivers/led.h"

#include "lib/STM32_USB_Device_Library/App/usb_device.h"
#include "lib/STM32_USB_Device_Library/Core/usbd_core.h"
#include "lib/STM32_USB_Device_Library/Class/MTP/usbd_mtp.h"

volatile bool msc_locked = false;
volatile bool msc_ejected = false;
volatile uint32_t msc_activity = 0;


bool msc_loop()
{
    INFO("USB mode on");
	led_set_backlight_timeout(GFX_BACKLIGHT_TIME);

    uint8_t start_up = false;

    bool usb_init = false;

    uint8_t old_charge = 0xFF;
    uint8_t old_data = 0xFF;
    pwr_data_mode_t old_data_mode = 0xFF;
    uint32_t next_update = 0;

    uint32_t pwr_delay = HAL_GetTick() + 4000;

    while (1)
    {
        pwr_step();



        if (button_pressed(BT1) ||button_pressed(BT2) || button_pressed(BT3) || button_pressed(BT4) || button_pressed(BT5))
        {
        	led_set_backlight(GFX_BACKLIGHT);
        	led_set_backlight_timeout(GFX_BACKLIGHT_TIME);
        }

        if ((pwr.data_port == PWR_DATA_CHARGE  || pwr.data_port == PWR_DATA_CHARGE_DONE) && !usb_init)
        {
            usb_init = true;
            msc_locked = false;
            MX_USB_DEVICE_Init();
        }

        if (msc_ejected)
        {
            start_up = true;
            break;
        }

        //get class data
        USBD_MTP_HandleTypeDef *hmtp = (USBD_MTP_HandleTypeDef *)hUsbDeviceHS.pClassDataCmsit;

        //are class data avalible (usb init ok)
        if (hmtp > 0)
        {
            if (pwr.data_port == PWR_DATA_CHARGE || pwr.data_port == PWR_DATA_CHARGE_DONE)
            {
                pwr.data_port = PWR_DATA_ACTIVE;
            }

            if (pwr.data_port == PWR_DATA_NONE)
            {
                usb_init = false;
                msc_locked = false;
                USBD_DeInit(&hUsbDeviceHS);
            }

            if (pwr.charge_port == PWR_CHARGE_NONE
            		&& old_charge > PWR_CHARGE_NONE)
            {
            	//after bq usb disconnection, restart usb stack
                pwr.data_port = PWR_DATA_NONE;
				usb_init = false;
				USBD_DeInit(&hUsbDeviceHS);
				msc_locked = false;
            }
        }
        else
        {
        	if (pwr.data_port == PWR_DATA_ACTIVE)
        	{
                pwr.data_port = PWR_DATA_NONE;
				usb_init = false;
				USBD_DeInit(&hUsbDeviceHS);
				msc_locked = false;
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
		if (pwr.charge_port == PWR_CHARGE_DONE && !msc_locked)
		{
			no_init->boot_type = BOOT_CHARGE;
			no_init_update();

			wait_anim_to_end = true;
		}

		//usb charging or active but msc unlocked
		if (button_hold(BT3) && !msc_locked)
		{
			//start right now!
			start_up = true;
			break;
		}

		//weak charger connected
		if (pwr.charge_port == PWR_CHARGE_WEAK
				&& !msc_locked
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

    if (usb_init)
    {
    	USBD_DeInit(&hUsbDeviceHS);
    }

    INFO("USB mode off");

	led_set_backlight(GFX_BACKLIGHT);
	led_set_backlight_timeout(0);

    return start_up;
}

//void msc_irq_handler()
//{
//	static uint8_t scsii_state_old = 0xFF;
//	pwr.data_usb_activity = HAL_GetTick();
//
//    //class data are avalible
//    if ((USBD_MSC_BOT_HandleTypeDef *)hUsbDeviceHS.pClassData > 0 && pwr.data_port == PWR_DATA_ACTIVE)
//    {
//    	uint8_t scsii_state = ((USBD_MSC_BOT_HandleTypeDef *)hUsbDeviceHS.pClassData)->scsi_medium_state;
//
//    	if (scsii_state_old != scsii_state)
//    	{
//    		INFO("scsii state %u %u", scsii_state, hUsbDeviceHS.dev_state);
//    		scsii_state_old = scsii_state;
//
//			if (scsii_state == SCSI_MEDIUM_EJECTED)
//			{
//				msc_ejected = true;
//			}
//
//			if (scsii_state == SCSI_MEDIUM_LOCKED)
//			{
//				msc_locked = true;
//			}
//
//			if (scsii_state == SCSI_MEDIUM_UNLOCKED)
//			{
//				msc_locked = false;
//			}
//    	}
//    }
//}
