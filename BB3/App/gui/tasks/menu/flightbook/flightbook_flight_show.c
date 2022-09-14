/*
 * flightbook_flight_show.c
 *
 * Show basic flight statistics, like 
 * time of start, landing, duration, max altitude...
 *
 *  Created on: Feb 27, 2022
 *      Author: tilmann@bubecks.de
 */

#define DEBUG_LEVEL	DEBUG_DBG

#include "flightbook_flight_show.h"

#include "gui/tasks/menu/settings.h"
#include "gui/tasks/menu/flightbook/flightbook.h"
#include "gui/tasks/menu/flightbook/flightbook_flight.h"
#include "gui/tasks/filemanager.h"

#include "gui/statusbar.h"
#include "gui/gui_list.h"
#include "gui/ctx.h"

#include "fc/logger/logger.h"
#include "fc/logger/igc.h"

#include "etc/format.h"
#include "etc/geo_calc.h"
#include "etc/epoch.h"

REGISTER_TASK_I(flightbook_flight_show);

static bool flight_show_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        //flightbook_flights_open_fm(true);
        return false;
    }
    return true;
}


static lv_obj_t * flightbook_flight_show_init(lv_obj_t * par)
{
	char value[100];
	flight_stats_t f_stat;

	DBG("flightbook_flight_show_init");

	filemanager_get_filename(value, flightbook_flight_filename);

	DBG("FILENAME: %s", value);

	lv_obj_t * list = gui_list_create(par, value, &gui_flightbook_flight, NULL);

	logger_read_flight_stats(flightbook_flight_filename, &f_stat);

	if (f_stat.start_time != FS_NO_DATA )
	{
		uint8_t sec, min, hour, day, wday, month;
		uint16_t year;
		datetime_from_epoch(f_stat.start_time, &sec, &min, &hour, &day, &wday, &month, &year);

		format_date(value, day, month, year);
		gui_list_text2_add_entry(list, "Date", value);

		format_time(value, hour, min);
		gui_list_text2_add_entry(list, "Start", value);

		if (f_stat.duration != FS_NO_DATA )
		{
			datetime_from_epoch(f_stat.start_time + f_stat.duration, &sec, &min, &hour, &day, &wday, &month, &year);
			format_time(value, hour, min);
			gui_list_text2_add_entry(list, "Landing", value);

			format_duration(value, (float)f_stat.duration);
			gui_list_text2_add_entry(list, "Duration", value);
		}
	}

	if (f_stat.max_alt != FS_NO_DATA )
	{
		format_altitude_with_units(value, (float)f_stat.max_alt);
		gui_list_text2_add_entry(list, "Altitude Max", value);
	}

	if (f_stat.min_alt != FS_NO_DATA )
	{
		format_altitude_with_units(value, (float)f_stat.min_alt);
		gui_list_text2_add_entry(list, "Altitude Min", value);
	}

	if (f_stat.max_climb != FS_NO_DATA )
	{
		format_vario_with_units(value, (float)f_stat.max_climb / 100.0);   // cm/s in m/s
		gui_list_text2_add_entry(list, "Max Raise", value);
	}

	if (f_stat.max_sink != FS_NO_DATA )
	{
		format_vario_with_units(value, (float)f_stat.max_sink / 100.0);  // cm/s in m/s
		gui_list_text2_add_entry(list, "Max Sink", value);
	}

	if (f_stat.odo != FS_NO_DATA )
	{
		format_distance_with_units(value, f_stat.odo / 100);     // cm to meter
		gui_list_text2_add_entry(list, "Track length", value);
	}

	return list;
}


