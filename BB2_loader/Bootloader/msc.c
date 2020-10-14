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
	bool usb_connected = false;

	pwr_step();

	if (pwr.data_port == PWR_DATA_ACTIVE)
	{
		usb_connected = true;

		INFO("USB mode on");
		gfx_draw_status(GFX_STATUS_CHARGE, NULL);

		MX_USB_DEVICE_Init();

		bool usb_in_use = false;

		while (1)
		{
			//get class data
			USBD_MSC_BOT_HandleTypeDef *hmsc = (USBD_MSC_BOT_HandleTypeDef *)hUsbDeviceHS.pClassData;

			//are class data avalible (usb init ok)
			if (hmsc > 0)
			{
				//update the screen
				if (!usb_in_use)
				{
					gfx_draw_status(GFX_STATUS_USB, NULL);
					usb_in_use = true;
				}

				//medium was ejected
				if (hmsc->scsi_medium_state == SCSI_MEDIUM_EJECTED)
				{
					break;
				}
			}

			//cable is disconnected
			if (HAL_GPIO_ReadPin(USB_DATA_DET) == LOW)
			{
				usb_connected = false;
				break;
			}
		}

		USBD_DeInit(&hUsbDeviceHS);
		INFO("USB mode off");
	}

	return usb_connected;
}
