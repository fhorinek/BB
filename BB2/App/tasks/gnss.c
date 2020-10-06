/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../common.h"

#include "../drivers/fanet.h"
#include "../drivers/gnss_ublox_m8.h"

#include "../config/config.h"


void task_GNSS(void *argument)
{
	vTaskSuspend(NULL);

	INFO("Started");


	ublox_init();

	fanet_init();


	for(;;)
	{
		ublox_step();

		fanet_step();

		neighbors_step();
	}
}
