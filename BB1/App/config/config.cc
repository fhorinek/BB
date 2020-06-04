/*
 * config.cc
 *
 *  Created on: May 4, 2020
 *      Author: horinek
 */
#include "config.h"

#include "fatfs.h"
#include "../debug.h"

//C linkage is necessary, because of way how you can initialize unions in C, not supported in C++
extern "C" {

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
		//devices
		{
			//gnss
			{
				//enabled
				entry_bool("gnss_en", true),
				//module
				entry_select("gnss_module", GNSS_MODULE_SIM, gnss_module_list),
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

}

void config_set_bool(cfg_entry_t * entry, bool val)
{
	entry->value.b = val;
}

bool config_get_bool(cfg_entry_t * entry)
{
	return entry->value.b;
}

void config_set_select(cfg_entry_t * entry, uint8_t val)
{
	cfg_entry_param_select_t * s;
	for(uint8_t i = 0; ; i++)
	{
		s = &entry->params.list[i];

		if (s->value == 0xFF)
			break;

		if (s->value == val)
		{
			entry->value.u8[0] = val;
			return;
		}
	}
}

uint8_t config_get_select(cfg_entry_t * entry)
{
	return entry->value.u8[0];
}


char * config_get_text(cfg_entry_t * entry)
{
	return entry->value.str;
}

void config_set_text(cfg_entry_t * entry, char * value)
{
	uint16_t len = min(strlen(value), entry->params.u16[0] - 1);
	memcpy(entry->value.str, value, len);
	entry->value.str[len] = 0;
}


void config_load()
{
	FIL f;
	uint8_t ret;

	char * path = SYSTEM_CONFIG_FILE;
	ret = f_open(&f, path, FA_READ);
	INFO("Reading configuration from %s", path);

	if (ret == FR_OK)
	{
		char buff[256];
		uint16_t line = 0;

		while (f_gets(buff, sizeof(buff), &f) != NULL)
		{
			line++;

			//remove \n
			buff[strlen(buff) - 1] = 0;

			char * pos = strchr(buff, '=');
			if (pos == NULL)
			{
				WARN("line %u: invalid format '%s'", line, buff);
				continue;
			}

			char key[64];
			memcpy(key, buff, pos - buff);
			key[pos - buff] = 0;


			cfg_entry_t * e = entry_find(key);
			if (e != NULL)
			{
				entry_set_str(e, pos + 1);
			}
			else
			{
				WARN("line %u: key '%s' not known", line, key);
			}
		}

		f_close(&f);
	}
	else
	{
		WARN("Unable to open");
	}
	INFO("Reading configuration done.");
}

void config_store()
{
	FIL f;
	uint8_t ret;

	char * path = SYSTEM_CONFIG_FILE;
	ret = f_open(&f, path, FA_WRITE | FA_CREATE_ALWAYS);
	INFO("Writing configuration to %s", path);

	if (ret != FR_OK)
	{
		INFO("Unable to open file %s", path);
		return;
	}

	uint16_t len = sizeof(config_t) / sizeof(cfg_entry_t);

	for (uint16_t i = 0; i < len; i++)
	{
		char buff[256];

		cfg_entry_t * entry = (cfg_entry_t *)(&config) + i;

		entry_get_str(buff, entry);
		UINT bw;
		//DBG("W %s", buff);
		f_write(&f, buff, strlen(buff), &bw);

	}

	f_close(&f);
}

