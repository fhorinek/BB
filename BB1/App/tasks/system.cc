/*
 * system.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../common.h"
#include "../app.h"
#include "../debug.h"

#include "../config/config.h"
#include "../drivers/sd.h"

extern "C" void task_System(void *argument);


void task_System(void *argument)
{
	INFO("Started");

	sd_init();

	vTaskResume((TaskHandle_t)DebugHandle);

	config_entry_init();
	config_load();
	//config_store();

	vTaskResume((TaskHandle_t)GUIHandle);
	vTaskResume((TaskHandle_t)GPSHandle);

	for(;;)
	{
		osDelay(10);


	}
}
