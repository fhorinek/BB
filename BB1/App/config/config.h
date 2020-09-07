/*
 * config.h
 *
 *  Created on: May 4, 2020
 *      Author: horinek
 */

#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_

#include "../common.h"
#include "entry.h"

#define ALTITUDE_M		0
#define ALTITUDE_FT		1

#define SPEED_KMH		0
#define SPEED_MPH		1
#define SPEED_MPS		2
#define SPEED_KNOTS		3

#define SPEED_KMH		0
#define SPEED_MPH		1
#define SPEED_MPS		2
#define SPEED_KNOTS		3

#define GNSS_DDdddddd	0
#define GNSS_DDMMmmm	1
#define GNSS_DDMMSS		2

#define EARTH_WGS84		0
#define EARTH_FAI		1

#define DISTANCE_METERS	0
#define DISTANCE_MILES	1

#define GNSS_MODULE_SIM	0
#define GNSS_MODULE_L96	1
#define GNSS_MODULE_UBL	2

#define PAGE_NAME_LEN	8
#define PAGE_MAX_COUNT	10

typedef struct
{
	struct
	{
		cfg_entry_t name;
	} pilot;

	struct
	{
		cfg_entry_t page[PAGE_MAX_COUNT];

	} ui;

	struct
	{
		struct
		{
			cfg_entry_t enabled;
			cfg_entry_t module;
			cfg_entry_t use_gps;
			cfg_entry_t use_glonass;
			cfg_entry_t use_galileo;
		} gnss;

		struct
		{
			cfg_entry_t enabled;
			cfg_entry_t broadcast_name;
			cfg_entry_t online_track;
		} fanet;
	} devices;

	struct
	{
		struct
		{
			cfg_entry_t backlight;
			cfg_entry_t backlight_timeout;
		} display;

		struct
		{
			cfg_entry_t altitude;
			cfg_entry_t speed;
			cfg_entry_t distance;
			cfg_entry_t geo_datum;
			cfg_entry_t earth_model;
		} units;
	} settings;

	struct
	{
		cfg_entry_t use_serial;
	} debug;
} config_t;

extern config_t config;

void config_load();
void config_store();
void config_default();
void config_show();

void config_set_bool(cfg_entry_t * entry, bool val);
bool config_get_bool(cfg_entry_t * entry);

void config_set_select(cfg_entry_t * entry, uint8_t val);
uint8_t config_get_select(cfg_entry_t * entry);

void config_set_text(cfg_entry_t * entry, char * value);
char * config_get_text(cfg_entry_t * entry);

int16_t config_get_int(cfg_entry_t * entry);
void config_set_int(cfg_entry_t * entry, int16_t value);

#endif /* CONFIG_CONFIG_H_ */
