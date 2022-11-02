/*
 * logger.h
 *
 *  Created on: Jun 30, 2021
 *      Author: horinek
 */

#ifndef FC_LOGGER_LOGGER_H_
#define FC_LOGGER_LOGGER_H_

#include "common.h"
#include "fc/fc.h"

#define FS_NO_DATA 0

/**
 * A structure holding all statistics of a flight stored in a log file.
 */
typedef struct flight_stats
{
	uint32_t start_time;    // in seconds since epoch UTC
	int32_t  tz_offset;     // timezone offset in seconds
	uint32_t duration;      // in seconds
	uint32_t odo;           // in cm

	int16_t max_alt;        // in m
	int16_t min_alt;        // in m

	int16_t max_climb;      // in cm/s
	int16_t max_sink;       // in cm/s

        int32_t min_lat, max_lat, min_lon, max_lon;
} flight_stats_t;

void logger_init();
void logger_start();
void logger_comment(const char *format, ...);
void logger_stop();
void logger_write_flight_stats(flight_stats_t f_stat);
void logger_read_flight_stats(const char *filename, flight_stats_t *f_stat);

fc_logger_status_t logger_state();

#endif /* FC_LOGGER_LOGGER_H_ */
