/*
 * default_cfg.c
 *
 *  Created on: 10. 8. 2020
 *      Author: horinek
 */

#include "config.h"
#include "entry.h"
#include "etc/timezone.h"
#include "etc/format.h"

cfg_entry_param_select_t date_format_select[] =
{
    {DATE_DDMMYYYY, "DMY"},
    {DATE_MMDDYYYY, "MDY"},
    {DATE_YYYYMMDD, "YMD"},
    SELECT_END
};

cfg_entry_param_select_t earth_model_select[] =
{
	{EARTH_WGS84, "WGS-84"},
	{EARTH_FAI, "FAI sphere"},
	SELECT_END
};

cfg_entry_param_select_t gdatum_select[] =
{
	{GNSS_DDdddddd, "DD.ddddd"},
	{GNSS_DDMMmmm, "DDMM.mmm"},
	{GNSS_DDMMSS, "DDMMSS"},
	SELECT_END
};

cfg_entry_param_select_t speed_select[] =
{
	{SPEED_KMH, "Kmh"},
	{SPEED_MPH, "Mph"},
	{SPEED_MPS, "Mps"},
	{SPEED_KNOTS, "Knots"},
	SELECT_END
};

cfg_entry_param_select_t distance_select[] =
{
	{DISTANCE_METERS, "Km"},
	{DISTANCE_MILES, "Mi"},
	SELECT_END
};

cfg_entry_param_select_t altitude_select[] =
{
	{ALTITUDE_M, "m"},
	{ALTITUDE_FT, "ft"},
	SELECT_END
};

cfg_entry_param_select_t vario_format_select[] =
{
    {VARIO_MPS, "mps"},
    {VARIO_KN, "kn"},
    {VARIO_FPM, "fpm"},
    SELECT_END
};

cfg_entry_param_select_t timezone_select[] =
{
	{UTC_n1200, "-1200"},
	{UTC_n1100, "-1100"},
	{UTC_n1000, "-1000"},
	{UTC_n0930, "-0930"},
	{UTC_n0900, "-0900"},
	{UTC_n0800, "-0800"},
	{UTC_n0700, "-0700"},
	{UTC_n0600, "-0600"},
	{UTC_n0500, "-0500"},
	{UTC_n0400, "-0400"},
	{UTC_n0330, "-0330"},
	{UTC_n0300, "-0300"},
	{UTC_n0200, "-0200"},
	{UTC_n0100, "-0100"},
	{UTC_p0000, "+0000"},
	{UTC_p0100, "+0100"},
	{UTC_p0200, "+0200"},
	{UTC_p0300, "+0300"},
	{UTC_p0330, "+0330"},
	{UTC_p0400, "+0400"},
	{UTC_p0430, "+0430"},
	{UTC_p0500, "+0500"},
	{UTC_p0530, "+0530"},
	{UTC_p0545, "+0545"},
	{UTC_p0600, "+0600"},
	{UTC_p0630, "+0630"},
	{UTC_p0700, "+0700"},
	{UTC_p0800, "+0800"},
	{UTC_p0845, "+0845"},
	{UTC_p0900, "+0900"},
	{UTC_p0930, "+0930"},
	{UTC_p1000, "+1000"},
	{UTC_p1030, "+1030"},
	{UTC_p1100, "+1100"},
	{UTC_p1200, "+1200"},
	{UTC_p1245, "+1245"},
	{UTC_p1300, "+1300"},
	{UTC_p1400, "+1400"},
	SELECT_END
};

cfg_entry_param_select_t firmware_channel_select[] =
{
    {FW_RELEASE, "release"},
    {FW_TESTING, "testing"},
    {FW_DEVEL, "devel"},
    SELECT_END
};

cfg_entry_param_select_t logger_mode_select[] =
{
    {LOGGER_OFF, "off"},
    {LOGGER_FLIGHT, "flight"},
    {LOGGER_ALWAYS, "always"},
    SELECT_END
};

cfg_entry_param_range_t qnh_range =
{
    .val_min.s32 = 80000,
    .val_max.s32 = 150000
};

cfg_entry_param_range_t acc_gain_range =
{
    .val_min.flt = 0.0,
    .val_max.flt = 1.0
};

pilot_profile_t pilot =
{
    //name
    entry_text("pilot_name", "Strato pilot", PILOT_NAME_LEN, 0),
    //broadcast_name
    entry_bool("bcst_name", true),
    //online track
    entry_bool("online_track", true),
};


flight_profile_t profile =
{
    //ui
    {
        //page
        {
            entry_text("page[0]", "default", PAGE_NAME_LEN, 0),
            entry_text("page[1]", "", PAGE_NAME_LEN, 0),
            entry_text("page[2]", "", PAGE_NAME_LEN, 0),
            entry_text("page[3]", "", PAGE_NAME_LEN, 0),
            entry_text("page[4]", "", PAGE_NAME_LEN, 0),
            entry_text("page[5]", "", PAGE_NAME_LEN, 0),
            entry_text("page[6]", "", PAGE_NAME_LEN, 0),
            entry_text("page[7]", "", PAGE_NAME_LEN, 0),
            entry_text("page[8]", "", PAGE_NAME_LEN, 0),
            entry_text("page[9]", "", PAGE_NAME_LEN, 0),
        },
        //page_last
        entry_int("page_last", 0, 0, PAGE_MAX_COUNT - 1),
    },

    //fanet
    {
        //enabled
        entry_bool("fa_en", true),
    },

    //flight
    {
        //auto_take_off
        {
            //alt_change_enabled
            entry_bool("auto_takeoff_alt", true),
            //alt_change_value
            entry_int("auto_takeoff_alt_val", 5, 1, 20),
            //speed_enabled
            entry_bool("auto_takeoff_speed", true),
            //speed_value
            entry_int("auto_takeoff_speed_val", 5, 1, 20),
            //timeout
            entry_int("auto_takeoff_timeout", 30, 1, 120),
        },
        //auto_landing
        {
            //alt_change_enabled
            entry_bool("auto_land_alt", true),
            //alt_change_value
            entry_int("auto_land_alt_val", 5, 1, 20),
            //speed_enabled
            entry_bool("auto_land_speed", true),
            //speed_value
            entry_int("auto_land_val", 5, 1, 20),
            //timeout
            entry_int("auto_land_timeout", 30, 1, 120),
        },
        //logger
        {
            //mode
            entry_select("log_mode", LOGGER_FLIGHT, logger_mode_select),
            //igc
            entry_bool("log_igc", true),
        }
    },

    //vario
    {
		//in_flight
		entry_bool("vario_in_flight", true),
		//sink
		entry_int("vario_sink", -50, -100, 100),
		//lift
		entry_int("vario_lift", 1, -100, 100),
        //acc_gain
        entry_float("vario_acc", 1.0, acc_gain_range),
        //avg_vario
        entry_int("vario_avg", 30, 5, 120),
        //profile
        entry_text("vario_profile", "default", VARIO_PROFILE_LEN, 0),
    },
};

config_t config =
{
    //device_name
    entry_text("dev_name", "", DEV_NAME_LEN, 0),
    //ask_on_start
    entry_bool("ask", true),
    //pilot profile
    entry_text("pilot", "pilot", DEV_NAME_LEN, 0),
    //flight profile
    entry_text("flight", "default", DEV_NAME_LEN, 0),

	//alt
	{
	    //qnh1
        entry_big_int("qnh1", 101325, qnh_range),
        //qnh2
        entry_big_int("qnh2", 101325, qnh_range),
	},

    //gnss
    {
        //use_gps
        entry_bool("gnss_gps", true),
        //use_glonass
        entry_bool("gnss_glonass", true),
        //use_galileo
        entry_bool("gnss_galileo", true),
    },

    //bluetooth
    {
        //a2dp
        entry_bool("bt_a2dp", true),
        //volume
        entry_int("bt_volume", 50, 0, 100),
        //name
        entry_text("bt_name", "Strato", BLUETOOTH_NAME_LEN, 0),
        //pin
        entry_text("bt_pin", "1234", BLUETOOTH_PIN_LEN, 0),
    },

    //wifi
    {
        //enabled
        entry_bool("wifi", true),
        //autoconnect
        entry_bool("wifi_auto", true),
        //ap
        entry_bool("wifi_ap", false),
        //ap_pass
        entry_text("wifi_ap_pass", "12345678", WIFI_PASS_LEN, 0),
    },

    //display
    {
        //backlight
        entry_int("disp_bckl", 80, 0, 100),
        //backlight_timeout
        entry_int("disp_bckl_time", 30, 10, 120),
    },

    //time
    {
        //zone
        entry_select("time_zone", UTC_p0000, timezone_select),
        //gnss_sync
        entry_bool("time_gnss_sync", true),
        //dst
        entry_bool("time_dst", false),
    },

    //units
    {
        //altitude
        entry_select("unit_alt", ALTITUDE_M, altitude_select),
        //speed
        entry_select("unit_spd", SPEED_KMH, speed_select),
        //distance
        entry_select("unit_dist", DISTANCE_METERS, distance_select),
        //geo_datum
        entry_select("unit_geo_dat", GNSS_DDdddddd, gdatum_select),
        //earth_model
        entry_select("unit_earth", EARTH_WGS84, earth_model_select),
        //time24
        entry_bool("unit_time24", true),
        //date
        entry_select("unit_date", DATE_DDMMYYYY, date_format_select),
        //vario
        entry_select("unit_vario", VARIO_MPS, vario_format_select)
    },

    //system
    {
        //server url
        entry_text("server_url", "https://strato.skybean.eu/update", UPDATE_URL_LEN, 0),
        //firmware_channel
        entry_select("fw_channel", FW_DEVEL, firmware_channel_select),
    },

	//debug
	{
		//use_serial
		entry_bool("dbg_serial", true),
		//use_file
		entry_bool("dbg_file", false),
	},
};
