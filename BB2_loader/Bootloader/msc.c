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

bool msc_loop()
{
    INFO("USB mode on");

    uint8_t start_up = false;

    bool usb_init = false;

    uint8_t old_charge = 0xFF;
    uint8_t old_data = 0xFF;

    while (1)
    {
        pwr_step();

        if (pwr.data_port == PWR_DATA_CHARGE && !usb_init)
        {
            usb_init = true;
            MX_USB_DEVICE_Init();
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


            //medium was ejected
            if (hmsc->scsi_medium_state == SCSI_MEDIUM_EJECTED)
            {
                USBD_DeInit(&hUsbDeviceHS);
                start_up = true;
                break;
            }
        }

        //change gfx status if needed
        if (old_charge != pwr.charge_port || old_data != pwr.data_port)
        {
            old_charge = pwr.charge_port;
            old_data = pwr.data_port;

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

        //no usb comunication and power button pressed
        if (pwr.data_port != PWR_DATA_ACTIVE && button_hold(BT3))
        {
            start_up = true;
            break;
        }

        //cable is disconnected
        if (pwr.charge_port == PWR_CHARGE_NONE && pwr.data_port == PWR_DATA_NONE)
        {
            break;
        }
    }


    INFO("USB mode off");

    return start_up;
}
