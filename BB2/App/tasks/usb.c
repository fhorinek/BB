/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../common.h"
#include "../debug.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"


void task_USB(void *argument)
{
	vTaskSuspend(NULL);

	INFO("Started\n");

	MX_USB_DEVICE_Init();

	for(;;)
	{
		uint8_t Buf[] = "Pojeb sa!\n";
		osDelay(1000);

		CDC_Transmit_HS(Buf, sizeof(Buf) - 1);

	}
}
