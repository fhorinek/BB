/*
 * system.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../common.h"
#include "../debug.h"

#include "fatfs.h"

#include "../config/config.h"
#include "../drivers/sd.h"
#include "../drivers/psram.h"
#include "../gui/widgets/pages.h"



#include "../lib/miniz/miniz.h"

static bool power_off = false;

void system_poweroff()
{
	power_off = true;
}

void task_System(void *argument)
{
	INFO("Started");

	GpioSetDirection(VCC_MAIN_EN, OUTPUT, GPIO_NOPULL);
	GpioWrite(VCC_MAIN_EN, HIGH);

	sd_init();

	vTaskResume((TaskHandle_t)DebugHandle);

	config_entry_init();

	config_load();
	pages_defragment();

	PSRAM_Init();

//	GpioSetDirection(ESP_BOOT_OPTION, INPUT);
//	GpioSetDirection(ESP_EN_RESET, OUTPUT);
//	GpioWrite(ESP_EN_RESET, LOW);
//
//	GpioSetDirection(ESP_SW_EN, OUTPUT);
//	GpioWrite(ESP_SW_EN, HIGH);
//	HAL_Delay(1000);
//	GpioSetDirection(ESP_EN_RESET, INPUT);
//
//	INFO("ESP pin OK");
//	vTaskResume((TaskHandle_t)GUIHandle);

/*
	uint32_t time_start = HAL_GetTick();
	FIL f_in, f_out;

	f_open(&f_in, "comp", FA_READ);
	f_open(&f_out, "decomp", FA_WRITE | FA_CREATE_NEW);

	uint32_t infile_size = f_size(&f_in);

	z_stream stream;

	#define BUF_SIZE (1024 * 32)
	static uint8_t s_inbuf[BUF_SIZE];
	static uint8_t s_outbuf[BUF_SIZE];

	memset(&stream, 0, sizeof(stream));
	stream.next_in = s_inbuf;
	stream.avail_in = 0;
	stream.next_out = s_outbuf;
	stream.avail_out = BUF_SIZE;

	uint32_t infile_remaining = infile_size;
	inflateInit(&stream);


	UINT br;
    for ( ; ; )
    {
      int8_t status;
      if (!stream.avail_in)
      {
        // Input buffer is empty, so read more bytes from input file.
        uint32_t n = min(BUF_SIZE, infile_remaining);


        f_read(&f_in, s_inbuf, n, &br);

        stream.next_in = s_inbuf;
        stream.avail_in = n;

        infile_remaining -= n;
      }

      status = inflate(&stream, Z_SYNC_FLUSH);

      if ((status == Z_STREAM_END) || (!stream.avail_out))
      {
        // Output buffer is full, or decompression is done, so write buffer to output file.
        uint n = BUF_SIZE - stream.avail_out;
        f_write(&f_out, s_outbuf, n, &br);

		stream.next_out = s_outbuf;
        stream.avail_out = BUF_SIZE;
      }

      if (status == Z_STREAM_END)
        break;
      else if (status != Z_OK)
      {
        printf("inflate() failed with status %i!\n", status);
      }
    }

	f_close(&f_in);
	f_close(&f_out);

	DBG("time: %lu", HAL_GetTick() - time_start);
*/

	fc_init();

	vTaskResume((TaskHandle_t)GUIHandle);
//	vTaskResume((TaskHandle_t)USBHandle);
//	vTaskResume((TaskHandle_t)MEMSHandle);
	vTaskResume((TaskHandle_t)GNSSHandle);


	for(;;)
	{
		osDelay(10);

		if (power_off)
			break;
	}

	INFO("Power down");

	//todo: deinit threads

	vTaskSuspend((TaskHandle_t)GUIHandle);
	vTaskSuspend((TaskHandle_t)USBHandle);
	vTaskSuspend((TaskHandle_t)MEMSHandle);
	vTaskSuspend((TaskHandle_t)GNSSHandle);

	//deinit fc

	//store configuration
	config_store();

	//unmount storage
	sd_deinit();

	osDelay(1000);

	//wait for option button to be release
	while(HAL_GPIO_ReadPin(BT2) == LOW);

	NVIC_SystemReset();
}
