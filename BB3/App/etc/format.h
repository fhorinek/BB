/*
 * format.h
 *
 *  Created on: 27. 5. 2020
 *      Author: horinek
 */

#ifndef ETC_FORMAT_H_
#define ETC_FORMAT_H_

#include "../common.h"

#define DATE_DDMMYYYY   0
#define DATE_MMDDYYYY   1
#define DATE_YYYYMMDD   2

extern char *month_names[12];

void format_date(char * buf, uint8_t day, uint8_t month, uint16_t year);
void format_date_DM(char * buff, uint8_t day, uint8_t month);
void format_date_epoch(char * buff, uint64_t epoch);
void format_time(char * buf, uint8_t hour, uint8_t min);

void format_gnss_datum(char * slat, char * slon, int32_t lat, int32_t lot);

void format_distance_with_units(char * buf, float in);
void format_distance(char * buf, float in);
void format_distance_units(char * buf, float in);
void format_distance_with_units2(char * buf, float in);

void format_altitude(char * buff, float in);
void format_altitude_units(char * buff);
void format_altitude_with_units(char * buff, float in);

void format_altitude_2(char * buff, float in, uint8_t units);
void format_altitude_units_2(char * buff, uint8_t units);
void format_altitude_with_units_2(char * buff, float in, uint8_t units);

void format_FL_with_altitude_with_units(char * buff, float in);

void format_altitude_gain_2(char * buff, float in, uint8_t units);
void format_altitude_gain(char * buff, float in);

void format_vario(char * val, float in);
void format_vario_units(char * units);
void format_vario_with_units(char * buff, float in);

void format_mac(char * buf, uint8_t * mac);
void format_ip(char * buf, uint8_t * ip);

void format_speed(char * val, float in);
void format_speed_units(char * units);
void format_speed_with_units(char * buff, float in);

void format_speed_2(char * val, float in, char * format);

void format_percent(char * buff, float in);
void format_duration(char * buff, float in);
void format_duration_sec_f(char * buff, float in);

void format_uuid(char * buff);
void format_int(char * buff, float in);

void format_hdg_to_points(char * buff, float in);

#endif /* ETC_FORMAT_H_ */
