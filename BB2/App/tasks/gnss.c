/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../common.h"
#include "../debug.h"

#include "../drivers/fanet.h"
#include "../fc/neighbors.h"
#include "../config/config.h"

#include "../drivers/gnss_ublox_m8.h"

void task_GNSS(void *argument)
{
	vTaskSuspend(NULL);

	INFO("Started");


	ublox_init();

	fanet_init();
	neighbors_init();

	for(;;)
	{
		ublox_step();

		fanet_step();

		neighbors_step();
	}
}
