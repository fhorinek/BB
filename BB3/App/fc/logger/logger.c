/*
 * logger.c
 *
 *  Created on: Jun 30, 2021
 *      Author: horinek
 */

#include "logger.h"

#include "fc/fc.h"
#include "config/config.h"

#include "igc.h"

void logger_init()
{
	igc_init();
}

void logger_start()
{
	if (config_get_bool(&profile.flight.logger.igc))
		igc_start();
}


void logger_stop()
{
	igc_stop();
}
