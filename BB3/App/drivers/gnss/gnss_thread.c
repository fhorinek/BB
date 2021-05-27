/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "gnss_thread.h"

#include "fanet.h"
#include "gnss_ublox_m8.h"

#include "fc/neighbors.h"


void thread_gnss_start(void *argument)
{
	INFO("Started");


	ublox_init();

	fanet_init();


	while (!system_power_off)
	{
		ublox_step();

		fanet_step();

		neighbors_step();

		osDelay(10);
	}

    INFO("Done");
    osThreadSuspend(thread_gnss);
}
