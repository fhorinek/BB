/*
 * msc.cc
 *
 *  Created on: 18. 6. 2020
 *      Author: horinek
 */


#include "msc.h"

#include "gfx.h"

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_msc.h"

#include "pwr_mng.h"

volatile bool msc_locked = false;
volatile bool msc_ejected = false;
volatile uint32_t msc_activity = 0;

bool msc_loop()
{
    INFO("USB mode on");

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

        if (pwr.data_port == PWR_DATA_CHARGE && !usb_init)
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
        USBD_MSC_BOT_HandleTypeDef *hmsc = (USBD_MSC_BOT_HandleTypeDef *)hUsbDeviceHS.pClassData;

        //are class data avalible (usb init ok)
        if (hmsc > 0)
        {
            if (pwr.data_port == PWR_DATA_CHARGE)
            {
                pwr.data_port = PWR_DATA_ACTIVE;
            }

            if (pwr.data_port == PWR_DATA_NONE)
            {
                usb_init = false;
                USBD_DeInit(&hUsbDeviceHS);
            }
        }
        else
        {
        	if (pwr.data_port == PWR_DATA_ACTIVE)
        	{
                pwr.data_port = PWR_DATA_NONE;
				usb_init = false;
				USBD_DeInit(&hUsbDeviceHS);
        	}
        }

        //change gfx status if needed
        if (old_charge != pwr.charge_port
        		|| old_data != pwr.data_port
				|| old_data_mode != pwr.data_usb_mode
				|| next_update < HAL_GetTick())
        {
            //to update battery percentage
            next_update = HAL_GetTick() + 2000;

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

                if (pwr.charge_port == PWR_CHARGE_NONE && pwr.data_port == PWR_DATA_CHARGE)
                    gfx_draw_status(GFX_STATUS_NONE_CHARGE, NULL);
            }

            //cable is disconnected
            if (pwr.charge_port == PWR_CHARGE_NONE && pwr.data_port == PWR_DATA_NONE)
            {
                gfx_draw_status(GFX_STATUS_NONE_NONE, NULL);
            }

        }

        bool anim_done = gfx_draw_anim();

        //usb active but device unlocked
        if (pwr.data_port == PWR_DATA_ACTIVE && !msc_locked && button_hold(BT3))
        {
            start_up = true;
            break;
        }

        //no usb comunication and power button pressed
        if (pwr.data_port != PWR_DATA_ACTIVE && button_hold(BT3))
        {
            start_up = true;
            break;
        }

        //no cable connected, animatin is done
        if (pwr.charge_port == PWR_CHARGE_NONE
        		&& pwr.data_port == PWR_DATA_NONE
				&& anim_done
				&& pwr_delay < HAL_GetTick())
        {
            break;
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

    return start_up;
}

void msc_irq_handler()
{
	static uint8_t scsii_state_old = 0xFF;
	pwr.data_usb_activity = HAL_GetTick();

    //class data are avalible
    if ((USBD_MSC_BOT_HandleTypeDef *)hUsbDeviceHS.pClassData > 0 && pwr.data_port == PWR_DATA_ACTIVE)
    {
    	uint8_t scsii_state = ((USBD_MSC_BOT_HandleTypeDef *)hUsbDeviceHS.pClassData)->scsi_medium_state;

    	if (scsii_state_old != scsii_state)
    	{
    		INFO("scsii state %u %u", scsii_state, hUsbDeviceHS.dev_state);
    		scsii_state_old = scsii_state;

			if (scsii_state == SCSI_MEDIUM_EJECTED)
			{
				msc_ejected = true;
			}

			if (scsii_state == SCSI_MEDIUM_LOCKED)
			{
				msc_locked = true;
			}

			if (scsii_state == SCSI_MEDIUM_UNLOCKED)
			{
				msc_locked = false;
			}
    	}
    }
}
