/*
 * default_cfg.c
 *
 *  Created on: 10. 8. 2020
 *      Author: horinek
 */

#include "config.h"
#include "entry.h"


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

cfg_entry_param_select_t gnss_module_list[] =
{
	{GNSS_MODULE_SIM, "sim"},
	{GNSS_MODULE_L96, "l96"},
	{GNSS_MODULE_UBL, "ubl"},
	SELECT_END
};

config_t config =
{
	//pilot
	{
		//name
		entry_text("pilot_name", "Boombox", 64, 0),
	},
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

	//devices
	{
		//gnss
		{
			//enabled
			entry_bool("gnss_en", true),
			//module
			entry_select("gnss_module", GNSS_MODULE_UBL, gnss_module_list),
			//use_gps
			entry_bool("gnss_gps", true),
			//use_glonass
			entry_bool("gnss_glonass", true),
			//use_galileo
			entry_bool("gnss_galileo", true),
		},
		//fanet
		{
			//enabled
			entry_bool("fa_en", false),
			//broadcast_name
			entry_bool("fa_bcst_name", true),
			//online track
			entry_bool("fa_online_track", true),
		},
	},
	//settings
	{
		//display
		{
				//backlight
				entry_int("disp_bckl", 80, 0, 100),
				//backlight_timeout
				entry_int("disp_bckl_time", 30, 10, 120),
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
		},
	},
	//debug
	{
		//use_serial
		entry_bool("dbg_serial", true),
	},
};
