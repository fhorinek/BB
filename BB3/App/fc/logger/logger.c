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

/**
 * printf-like function to send output to the GPS log .
 *
 * \param format a printf-like format string 
 **/ 
void logger_comment(const char *format, ...)
{
 	va_list arp;
 	char text[80];

  	va_start(arp, format);
 	vsnprintf(text, sizeof(text), format, arp);
 	va_end(arp);

  	igc_comment(text);
    // Add additional loggers here, if available.
}

void logger_stop()
{
	igc_stop();
	csv_stop();
}
