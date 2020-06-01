/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../common.h"
#include "../app.h"
#include "../debug.h"

#include "../drivers/gnss_sim33ela.h"
#include "../drivers/fanet.h"

extern "C" void task_GPS(void *argument);


void task_GPS(void *argument)
{
	INFO("Started");

	//vTaskSuspend(NULL);

	gnss_init();
	fanet_init();

	for(;;)
	{
		gnss_step();

		fanet_step();
	}
}
