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

#define VARIO_MPS       0
#define VARIO_KN        1
#define VARIO_FPM       2

#define GNSS_DDdddddd	0
#define GNSS_DDMMmmm	1
#define GNSS_DDMMSS		2

#define EARTH_WGS84		0
#define EARTH_FAI		1

#define DISTANCE_METERS	0
#define DISTANCE_MILES	1

#define DEV_NAME_LEN   16

#define PAGE_NAME_LEN	16
#define PAGE_MAX_COUNT	10

#define BLUETOOTH_NAME_LEN      16
#define BLUETOOTH_PIN_LEN       4

typedef struct
{
    cfg_entry_t device_name;

	struct
	{
		cfg_entry_t name;
	} pilot;

	struct
	{
		cfg_entry_t page[PAGE_MAX_COUNT];
		cfg_entry_t page_last;
	} ui;

	struct
	{
        cfg_entry_t qnh1;
        cfg_entry_t qnh2;
        cfg_entry_t acc_gain;
        cfg_entry_t avg_duration;
	} vario;

    struct
    {
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

    struct
    {
        cfg_entry_t a2dp;
        cfg_entry_t volume;
        cfg_entry_t name;
        cfg_entry_t pin;
    } bluetooth;

    struct
    {
        cfg_entry_t enabled;
        cfg_entry_t ap;
    } wifi;

    struct
    {
        cfg_entry_t backlight;
        cfg_entry_t backlight_timeout;
    } display;

    struct
    {
        cfg_entry_t zone;
        cfg_entry_t sync_gnss;
        cfg_entry_t dst;
    } time;

    struct
    {
        cfg_entry_t altitude;
        cfg_entry_t speed;
        cfg_entry_t distance;
        cfg_entry_t geo_datum;
        cfg_entry_t earth_model;
        cfg_entry_t time24;
        cfg_entry_t date;
        cfg_entry_t vario;
    } units;

	struct
	{
		cfg_entry_t use_serial;
	} debug;
} config_t;

typedef void (* cfg_cb_t)(cfg_entry_t *);

typedef struct
{
    cfg_entry_t * entry;
    cfg_cb_t cb;
} cfg_callback_pair_t;

extern config_t config;
extern cfg_callback_pair_t config_callbacks[];

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

int32_t config_get_big_int(cfg_entry_t * entry);
void config_set_big_int(cfg_entry_t * entry, int32_t value);

float config_get_float(cfg_entry_t * entry);
void config_set_float(cfg_entry_t * entry, float value);


void config_process_cb(cfg_entry_t * entry);
void config_trigger_callbacks();
void config_disable_callbacks();
void config_enable_callbacks();


#endif /* CONFIG_CONFIG_H_ */
