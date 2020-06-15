/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../common.h"
#include "../app.h"
#include "../debug.h"

#include "../drivers/fanet.h"
#include "../fc/neighbors.h"
#include "../config/config.h"

#include "../drivers/gnss_sim33ela.h"
#include "../drivers/gnss_l96.h"
#include "../drivers/gnss_ublox_m8.h"

extern "C" void task_GPS(void *argument);


void task_GPS(void *argument)
{
	vTaskSuspend(NULL);

	INFO("Started");

	switch (config_get_select(&config.devices.gnss.module))
	{
		case(GNSS_MODULE_SIM):
			sim33ela_init();
		break;
		case(GNSS_MODULE_L96):
			l96_init();
		break;
		case(GNSS_MODULE_UBL):
			ublox_m8_init();
		break;
	}

	fanet_init();
	neighbors_init();

	for(;;)
	{
		switch (config_get_select(&config.devices.gnss.module))
		{
			case(GNSS_MODULE_SIM):
				sim33ela_step();
			break;
			case(GNSS_MODULE_L96):
				l96_step();
			break;
			case(GNSS_MODULE_UBL):
				ublox_m8_step();
			break;
		}

		fanet_step();

		neighbors_step();
	}
}
