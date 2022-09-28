/*
 * igc.h
 *
 *  Created on: Jun 30, 2021
 *      Author: horinek
 */

#ifndef FC_LOGGER_IGC_H_
#define FC_LOGGER_IGC_H_

#include "common.h"
#include "logger.h"

void igc_init();
void igc_start();
void igc_stop();
void igc_comment(char * text);

typedef struct flight_pos
{
	uint64_t timestamp;
	int32_t lat, lon;
	int16_t gnss_alt;
	int16_t baro_alt;

} flight_pos_t;

bool igc_read_next_pos(int32_t igc_log_read_file, flight_pos_t *flight_pos);
void igc_read_flight_stats(int32_t fp, flight_stats_t *f_stat);

extern int32_t my_lat, my_lon;

#endif /* FC_LOGGER_IGC_H_ */
