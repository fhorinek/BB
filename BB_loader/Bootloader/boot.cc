/*
 * boot.cc
 *
 *  Created on: Jun 10, 2020
 *      Author: horinek
 */

#include "common.h"

extern "C" void app_main();

#include "drivers/tft_hx8352.h"

#include "img/usb.c"

#define BOOT_STATUS_USB	0


void draw_status(uint8_t status)
{
	static bool disp_init = false;

	if (!disp_init)
	{
		tft_init();
		tft_init_display();
		tft_color_fill(0);
//		tft_test_pattern();
		tft_refresh_buffer(0, 0, 239, 399);

		disp_init = true;
	}

	switch (status)
	{
		case(BOOT_STATUS_USB):
			{

			uint16_t x1 = (TFT_WIDTH - img_usb.width) / 2;
			uint16_t y1 = (TFT_HEIGHT - img_usb.height) / 2;
			uint16_t x2 = x1 + img_usb.width - 1;
			uint16_t y2 = y1 + img_usb.height - 1;

			tft_wait_for_buffer();
			memcpy(tft_buffer, img_usb.pixel_data, sizeof(img_usb.pixel_data));
			tft_refresh_buffer(x1, y1, x2, y2);
			}
		break;

	}
}


#include "usb_device.h"

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_msc.h"

void app_msc()
{

	extern USBD_HandleTypeDef hUsbDeviceFS;
	USBD_MSC_BOT_HandleTypeDef *hmsc = (USBD_MSC_BOT_HandleTypeDef *)hUsbDeviceFS.pClassData;

	//usb init TODO: if cable is connected
	//time out when no cable is connected?


	INFO("USB mode on");
	MX_USB_DEVICE_Init();

	draw_status(BOOT_STATUS_USB);

	while (1)
	{

		if (hmsc->scsi_medium_state == SCSI_MEDIUM_EJECTED)
			break;
	}

	INFO("USB mode off");
	USBD_DeInit(&hUsbDeviceFS);

}

void app_main()
{
	INFO("Bootloader start");

	//SD Card init
	GpioSetDirection(CDMMC1_SW_EN, OUTPUT);
	GpioWrite(CDMMC1_SW_EN, HIGH);


	app_msc();
}
