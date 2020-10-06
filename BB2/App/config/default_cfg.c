/*
 * default_cfg.c
 *
 *  Created on: 10. 8. 2020
 *      Author: horinek
 */

#include "config.h"
#include "entry.h"
#include "../etc/timezone.h"

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
			entry_bool("fa_en", true),
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

		//time
		{
			//zone
			entry_select("time_zone", UTC_p0000, timezone_select),
			//gnss_sync
			entry_bool("time_gnss_sync", true),

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
