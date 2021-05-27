/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "common.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"


void thread_usb_start(void *argument)
{
	INFO("Started\n");

	MX_USB_DEVICE_Init();

	while (!system_power_off)
	{
		uint8_t Buf[] = "Pojeb sa!\n";
		osDelay(1000);

		CDC_Transmit_HS(Buf, sizeof(Buf) - 1);

	}

    INFO("Done");
    osThreadSuspend(thread_usb);
}
