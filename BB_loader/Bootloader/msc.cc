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
	INFO("USB mode on");
	MX_USB_DEVICE_Init();

	extern USBD_HandleTypeDef hUsbDeviceFS;

	//usb init TODO: if cable is connected
	//time out when no cable is connected?


	//gfx_draw_status(GFX_STATUS_CHARGE);

	gfx_draw_status(GFX_STATUS_USB);

	while (1)
	{
		//break;
		USBD_MSC_BOT_HandleTypeDef *hmsc = (USBD_MSC_BOT_HandleTypeDef *)hUsbDeviceFS.pClassData;

		//eject!
		if (hmsc->scsi_medium_state == SCSI_MEDIUM_EJECTED)
			break;
	}

	USBD_DeInit(&hUsbDeviceFS);
	INFO("USB mode off");

	return true;
}
