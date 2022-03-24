/*
 * config.h
 *
 *  Created on: May 4, 2020
 *      Author: horinek
 */

#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_

#include "common.h"
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

#define GALT_ELLIPSOID   0
#define GALT_MSL      	1

#define EARTH_WGS84		0
#define EARTH_FAI		1

#define DISTANCE_METERS	0
#define DISTANCE_MILES	1

#define DEV_NAME_LEN   16

#define PAGE_NAME_LEN       16
#define VARIO_PROFILE_LEN   16
#define PAGE_MAX_COUNT	    10

#define PILOT_NAME_LEN		64

#define BLUETOOTH_NAME_LEN      16
#define BLUETOOTH_PIN_LEN       8

#define WIFI_PASS_LEN           64
#define UPDATE_URL_LEN          128

#define FW_RELEASE  0
#define FW_TESTING  1
#define FW_DEVEL    2

#define LOGGER_OFF      0
#define LOGGER_FLIGHT   1
#define LOGGER_ALWAYS   2

#define DBG_TASK_NONE   0
#define DBG_TASK_ESP    1
#define DBG_TASK_STM    2

#define MAP_ALT_RANGE_FLAT      0
#define MAP_ALT_RANGE_NORMAL    1
#define MAP_ALT_RANGE_ALPS      2

typedef struct
{
    cfg_entry_t name;
    cfg_entry_t glider_type;
    cfg_entry_t glider_id;
    cfg_entry_t broadcast_name;
    cfg_entry_t online_track;
} pilot_profile_t;

typedef struct
{
    struct
    {
        cfg_entry_t page[PAGE_MAX_COUNT];
        cfg_entry_t page_last;
        cfg_entry_t last_lon;
        cfg_entry_t last_lat;

        struct
        {
        	cfg_entry_t power_on;
        	cfg_entry_t take_off;
        	cfg_entry_t circle;
        	cfg_entry_t glide;
        	cfg_entry_t land;
        } autoset;
    } ui;

    struct
    {
        cfg_entry_t enabled;
        cfg_entry_t flarm;
        cfg_entry_t air_type;
        cfg_entry_t ground_type;
    } fanet;

    struct
    {
        struct
        {
            cfg_entry_t alt_change_enabled;
            cfg_entry_t alt_change_value;
            cfg_entry_t speed_enabled;
            cfg_entry_t speed_value;
            cfg_entry_t timeout;
        } auto_take_off;

        struct
        {
            cfg_entry_t alt_change_enabled;
            cfg_entry_t alt_change_value;
            cfg_entry_t speed_enabled;
            cfg_entry_t speed_value;
            cfg_entry_t timeout;
        } auto_landing;

        struct
        {
            cfg_entry_t mode;
            cfg_entry_t igc;
            cfg_entry_t csv;
        } logger;

        cfg_entry_t acc_duration;
        cfg_entry_t gr_duration;
        cfg_entry_t circle_timeout;
    }
    flight;

    struct
    {
        cfg_entry_t in_flight;
        cfg_entry_t sink;
        cfg_entry_t lift;
        cfg_entry_t acc_gain;
        cfg_entry_t avg_duration;
        cfg_entry_t profile;
    } vario;

    struct
	{
        cfg_entry_t zoom_flight;
        cfg_entry_t blur;
        cfg_entry_t alt_range;
        cfg_entry_t show_fanet;
	} map;

	struct
	{
        cfg_entry_t a2dp_volume;
        cfg_entry_t sound_volume;
        cfg_entry_t vario_volume;
        cfg_entry_t master_volume;
        cfg_entry_t tts_alerts;
	} audio;

    struct
    {
        cfg_entry_t enabled;
        cfg_entry_t a2dp;
        cfg_entry_t spp;
        cfg_entry_t ble;
        cfg_entry_t protocol;
        cfg_entry_t forward_gnss;
    } bluetooth;

    struct
    {
        cfg_entry_t enabled;
        cfg_entry_t autoconnect;
        cfg_entry_t ap;
    } wifi;

} flight_profile_t;

typedef struct
{
    cfg_entry_t device_name;
    cfg_entry_t ask_on_start;
    cfg_entry_t pilot_profile;
    cfg_entry_t flight_profile;

	struct
	{
        cfg_entry_t qnh1;
        cfg_entry_t qnh2;
	} vario;

    struct
    {
        cfg_entry_t pin;
    } bluetooth;

    struct
    {
        cfg_entry_t ap_pass;
    } wifi;

    struct
    {
        cfg_entry_t backlight;
        cfg_entry_t backlight_timeout;
        cfg_entry_t bat_per;
        cfg_entry_t page_anim;
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
        cfg_entry_t galt;
        cfg_entry_t time24;
        cfg_entry_t date;
        cfg_entry_t vario;
    } units;

    struct
    {
        cfg_entry_t server_url;
        cfg_entry_t fw_channel;
        cfg_entry_t check_for_updates;
    } system;

	struct
	{
		cfg_entry_t use_serial;
		cfg_entry_t use_file;
		cfg_entry_t use_usb;
		cfg_entry_t esp_off;
        cfg_entry_t esp_wdt;
        cfg_entry_t tasks;
        cfg_entry_t vario_random;
        cfg_entry_t lvgl_info;
        cfg_entry_t fanet_update;
        cfg_entry_t crash_dump;
	} debug;
} config_t;

typedef void (* cfg_cb_t)(cfg_entry_t *);

typedef struct
{
    cfg_entry_t * entry;
    cfg_cb_t cb;
} cfg_callback_pair_t;

extern config_t config;
extern flight_profile_t profile;
extern pilot_profile_t pilot;

extern cfg_callback_pair_t config_callbacks[];

void config_load(cfg_entry_t * structure, char * path);
void config_store(cfg_entry_t * structure, char * path);
void config_show(cfg_entry_t * structure);

void config_set_bool(cfg_entry_t * entry, bool val);
bool config_get_bool(cfg_entry_t * entry);

void config_set_select(cfg_entry_t * entry, uint8_t val);
uint8_t config_get_select(cfg_entry_t * entry);
uint8_t config_get_select_at_index(cfg_entry_t * entry, uint8_t index);
uint8_t config_get_select_index(cfg_entry_t * entry);
const char * config_get_select_text(cfg_entry_t * entry);
const char * config_get_select_text_at_index(cfg_entry_t * entry, uint8_t index);
uint8_t config_get_select_cnt(cfg_entry_t * entry);

void config_set_text(cfg_entry_t * entry, char * value);
char * config_get_text(cfg_entry_t * entry);
uint16_t config_text_max_len(cfg_entry_t * entry);

int16_t config_get_int(cfg_entry_t * entry);
void config_set_int(cfg_entry_t * entry, int16_t value);
int16_t config_int_max(cfg_entry_t * entry);
int16_t config_int_min(cfg_entry_t * entry);

int32_t config_get_big_int(cfg_entry_t * entry);
void config_set_big_int(cfg_entry_t * entry, int32_t value);

float config_get_float(cfg_entry_t * entry);
void config_set_float(cfg_entry_t * entry, float value);
float config_float_max(cfg_entry_t * entry);
float config_float_min(cfg_entry_t * entry);

void config_process_cb(cfg_entry_t * entry);
void config_trigger_callbacks();
void config_disable_callbacks();
void config_enable_callbacks();
uint16_t config_structure_size(cfg_entry_t * structure);

void config_load_all();
void config_store_all();
void config_restore_factory();

void config_change_pilot(char * pilot_name);
void config_change_profile(char * profile_name);
uint8_t config_profiles_cnt();

void config_new_version_cb();

extern bool config_changed;

#endif /* CONFIG_CONFIG_H_ */
