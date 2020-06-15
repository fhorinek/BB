/*
 * boot.cc
 *
 *  Created on: Jun 10, 2020
 *      Author: horinek
 */

#include "common.h"

extern "C" void app_main();

void app_main()
{
	INFO("Bootloader start");

	HAL_Delay(100);

	INFO("Delay");

	GpioSetDirection(CDMMC1_SW_EN, OUTPUT);
	GpioWrite(CDMMC1_SW_EN, HIGH);
}
