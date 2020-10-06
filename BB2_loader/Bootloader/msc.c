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


bool msc_loop()
{

	//usb init TODO: if cable is connected
	//time out when no cable is connected?

	//gfx_draw_status(GFX_STATUS_CHARGE);

	GpioSetDirection(PA9, INPUT, GPIO_NOPULL);

	if (HAL_GPIO_ReadPin(PA9) == HIGH)
	{
		INFO("USB mode on");
		gfx_draw_status(GFX_STATUS_CHARGE, NULL);

		MX_USB_DEVICE_Init();

		bool usb_in_use = false;

		while (1)
		{
			USBD_MSC_BOT_HandleTypeDef *hmsc = (USBD_MSC_BOT_HandleTypeDef *)hUsbDeviceHS.pClassData;

			//eject!
			if (hmsc > 0)
			{
				if (!usb_in_use)
				{
					gfx_draw_status(GFX_STATUS_USB, NULL);
					usb_in_use = true;
				}

				if (hmsc->scsi_medium_state == SCSI_MEDIUM_EJECTED)
				{
					break;
				}
			}

			if (HAL_GPIO_ReadPin(PA9) == LOW)
				break;
		}

		USBD_DeInit(&hUsbDeviceHS);
		INFO("USB mode off");
	}

	return true;
}
