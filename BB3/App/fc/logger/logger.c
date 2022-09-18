/*
 * logger.c
 *
 *  Created on: Jun 30, 2021
 *      Author: horinek
 */

#include "logger.h"

#include <inttypes.h>

#include "fc/fc.h"
#include "config/config.h"

#include "igc.h"
#include "csv.h"

fc_logger_status_t logger_state()
{
	return (fc.logger.igc > fc.logger.csv ? fc.logger.igc : fc.logger.csv);
}

/**
 * Write the given flight statistics into the log file by using
 * special keywords inside comments.
 *
 * @param f_stat the flight statistics to save
 */
void logger_write_flight_stats(flight_stats_t f_stat)
{
	logger_comment(" SKYBEAN-START-UTC-s: %" PRIu32, f_stat.start_time);
	logger_comment(" SKYBEAN-DURATION-s: %" PRIu32, f_stat.duration);
 	logger_comment(" SKYBEAN-ALT-MAX-m: %" PRId16, f_stat.max_alt);
 	logger_comment(" SKYBEAN-ALT-MIN-m: %" PRId16, f_stat.min_alt);
 	logger_comment(" SKYBEAN-CLIMB-MAX-cm: %" PRId16, f_stat.max_climb);
 	logger_comment(" SKYBEAN-SINK-MAX-cm: %" PRId16, f_stat.max_sink);
 	logger_comment(" SKYBEAN-ODO-m: %" PRIu32, f_stat.odo);
}

/**
 * Read the flight statistics from a log file. This is either done by
 * searching for the keywords and values or by parsing the IGC file,
 * if no keywords are found (for old IGC logs).
 *
 * @param filename the filename of the log file to parse
 * @param f_stat a pointer to the flight statistics to store the values
 */
void logger_read_flight_stats(const char *filename, flight_stats_t *f_stat)
{
	int32_t fp;
	char line[80];
	char *p;

	// Set defaults, if nothing could be found in the file:
	f_stat->start_time = FS_NO_DATA;
	f_stat->duration = FS_NO_DATA;
	f_stat->max_alt = FS_NO_DATA;
	f_stat->min_alt = FS_NO_DATA;
	f_stat->max_climb = FS_NO_DATA;
	f_stat->max_sink = FS_NO_DATA;
	f_stat->odo = FS_NO_DATA;

	fp = red_open(filename, RED_O_RDONLY);
	if ( fp < 0 ) return;

	red_lseek(fp, -512, RED_SEEK_END);    	// Read from the end of the file

	while(1)
	{
		if ( file_gets(line, sizeof(line), fp) == NULL ) break;

		p = strstr(line, "SKYBEAN-START-UTC-s: ");
		if ( p != NULL )
		{
			f_stat->start_time = atol(p + 17);
			continue;
		}

		p = strstr(line, "SKYBEAN-DURATION-s: ");
		if ( p != NULL )
		{
			f_stat->duration = atol(p + 21);
			continue;
		}

		p = strstr(line, "SKYBEAN-ALT-MAX-m: ");
		if ( p != NULL )
		{
			f_stat->max_alt = atoi(p + 19);
			continue;
		}

		p = strstr(line, "SKYBEAN-ALT-MIN-m: ");
		if ( p != NULL )
		{
			f_stat->min_alt = atoi(p + 19);
			continue;
		}

		p = strstr(line, "SKYBEAN-CLIMB-MAX-cm: ");
		if ( p != NULL )
		{
			f_stat->max_climb = atoi(p + 22);
			continue;
		}

		p = strstr(line, "SKYBEAN-SINK-MAX-cm: ");
		if ( p != NULL )
		{
			f_stat->max_sink = atoi(p + 21);
			continue;
		}

		p = strstr(line, "SKYBEAN-ODO-m: ");
		if ( p != NULL )
		{
			f_stat->odo = atol(p + 16) * 100;   // meter in cm
			continue;
		}
	}

	if (f_stat->start_time == FS_NO_DATA)
	{
		// Fallback: this is an old file without comments, so read data out of files
		red_lseek(fp, 0, RED_SEEK_SET);
		igc_read_flight_stats(fp, f_stat);
	}
	red_close(fp);
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
