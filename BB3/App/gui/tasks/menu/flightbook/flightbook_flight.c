/*
 * flightbook_flight.c
 *
 * Show basic flight statistics, like 
 * time of start, landing, duration, max altitude...
 *
 *  Created on: Feb 27, 2022
 *      Author: tilmann@bubecks.de
 */

#define DEBUG_LEVEL	DEBUG_DBG

#include <inttypes.h>
#include "flightbook_flight.h"
#include "flightbook_flight_map.h"

#include "gui/tasks/menu/settings.h"
#include "gui/tasks/menu/flightbook/flightbook.h"
#include "gui/tasks/menu/flightbook/flightbook_flight.h"
#include "gui/tasks/filemanager.h"

#include "gui/dialog.h"
#include "gui/gui_list.h"

#include "fc/logger/logger.h"
#include "fc/logger/igc.h"

#include "etc/format.h"
#include "etc/geo_calc.h"
#include "etc/epoch.h"

REGISTER_TASK_I(flightbook_flight,
	char fm_return_path[PATH_LEN];
	uint8_t fm_return_level;
);

static bool flightbook_flight_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
    if (event == LV_EVENT_CANCEL)
    {
        gui_switch_task(&gui_filemanager, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
        filemanager_open(local->fm_return_path, local->fm_return_level, &gui_settings, FM_FLAG_FOCUS | FM_FLAG_SORT_DATE | FM_FLAG_SHOW_EXT, flightbook_flights_fm_cb);
        return false;
    }
    return true;
}

void flightbook_flight_read_task(char * path)
{
	char value[64];
	flight_stats_t f_stat;

	DBG("flightbook_flight_read_task");

	filemanager_get_filename(value, path);

	lv_obj_t * list = gui.list.list;

	gui_list_set_title(list, value);

	DBG("FILENAME: %s", value);

	logger_read_flight_stats(path, &f_stat);
	free(path);

	gui_lock_acquire();

	if (f_stat.start_time != FS_NO_DATA)
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

	gui_lock_release();

	dialog_close();

    vTaskDelete(NULL);
}

void flightbook_flight_open(char * path, uint8_t fm_level)
{
	strcpy(flightbook_flight_map_path, path);

	filemanager_get_path(local->fm_return_path, path);
	local->fm_return_level = fm_level;

	char file[64];
	filemanager_get_filename(file, path);
	dialog_show("Analysing...", file, dialog_progress, NULL);
	dialog_progress_spin();

	//need to allocate memory for path
	// input parameter *path of this function will be reused, before the new task start
	// flightbook_flight_read_task will need to free it
	char * tmp_path = (char *)malloc(strlen(path) + 1);
	strcpy(tmp_path, path);
	xTaskCreate((TaskFunction_t)flightbook_flight_read_task, "fb_read_task", 1024 * 2, tmp_path, osPriorityIdle + 1, NULL);
}


static lv_obj_t * flightbook_flight_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "", NULL, flightbook_flight_cb);

	gui_list_auto_entry(list, "Show on map", NEXT_TASK, &gui_flightbook_flight_map);

	return list;
}


