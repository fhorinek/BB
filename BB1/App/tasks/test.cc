/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../common.h"
#include "../app.h"
#include "../debug.h"

#include "fatfs.h"


extern "C" void task_Test(void *argument);

FIL f;

void task_Test(void *argument)
{
	vTaskSuspend(NULL);

	uint8_t res;

	INFO("Started");


	GpioSetDirection(CDMMC1_SW_EN, OUTPUT);
	GpioWrite(CDMMC1_SW_EN, HIGH);

	res =  f_mount(&SDFatFS, SDPath, 1);
	DBG("res = %u\n", res);

	res = f_open(&f, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
	DBG("res = %u\n", res);

	char buf[] = "alksdfuhbalsiudhfilasuhdf";

	UINT bw;
	res = f_write(&f, buf, strlen(buf), &bw);
	DBG("bw = %u\n", bw);
	DBG("res = %u\n", res);

	res = f_close(&f);
	DBG("res = %u\n", res);

	for(;;)
	{



	}
}
