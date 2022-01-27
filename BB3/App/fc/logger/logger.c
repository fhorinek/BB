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
#include "csv.h"

fc_logger_status_t logger_state()
{
	return (fc.logger.igc > fc.logger.csv ? fc.logger.igc : fc.logger.csv);
}


void logger_init()
{
	igc_init();
	csv_init();
}

void logger_start()
{
	if (config_get_bool(&profile.flight.logger.igc))
		igc_start();

	if (config_get_bool(&profile.flight.logger.csv))
		csv_start();
}


void logger_stop()
{
	igc_stop();
	csv_stop();
}
