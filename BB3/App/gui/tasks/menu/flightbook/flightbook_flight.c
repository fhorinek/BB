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
	char file_path[PATH_LEN];
	uint8_t fm_return_level;
);

//this overload the standard gui list function, so we can return correct filemanager level
static bool flightbook_flight_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
    if (event == LV_EVENT_CANCEL)
    {
        gui_switch_task(&gui_filemanager, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
        char return_path[PATH_LEN];
        filemanager_get_path(return_path, local->file_path);
        filemanager_open(return_path, local->fm_return_level, &gui_settings, FM_FLAG_FOCUS | FM_FLAG_SORT_DATE | FM_FLAG_SHOW_EXT, flightbook_flights_fm_cb);

        //do not handle cancel event
        return false;
    }

    //other events handle normally
    return true;
}

void flightbook_flight_read_task(char * param)
{
	char value[64];
	flight_stats_t f_stat;

	DBG("flightbook_flight_read_task");

	filemanager_get_filename(value, local->file_path);

	lv_obj_t * list = gui.list.list;

	gui_list_set_title(list, value);

	DBG("FILENAME: %s", value);

	logger_read_flight_stats(local->file_path, &f_stat);

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
	gui_low_priority(false);

	RedTaskUnregister();
    vTaskDelete(NULL);
}

void flightbook_flight_open(char * path, uint8_t fm_level)
{
	strcpy(local->file_path, path);
	local->fm_return_level = fm_level;

	char file[REDCONF_NAME_MAX];
	filemanager_get_filename(file, path);
	dialog_show("Analysing...", file, dialog_progress, NULL);
	dialog_progress_spin();
	gui_low_priority(true);

	xTaskCreate((TaskFunction_t)flightbook_flight_read_task, "fb_read_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);
}

static bool flightbook_flight_map_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
	if (event == LV_EVENT_PRESSED)
	{
		//this is standard method how to pass extra parameter to task
		//1. Switch to task, so the local memory is allocated for the new task
		gui_switch_task(&gui_flightbook_flight_map, LV_SCR_LOAD_ANIM_MOVE_LEFT);
		//2. Call custom function specific to target task to pass parameters
		flightbook_flight_map_load(local->file_path, local->fm_return_level);

		return false;
	}
	return true;
}

static lv_obj_t * flightbook_flight_init(lv_obj_t * par)
{
    help_set_base("Flightbook/Flight");

	lv_obj_t * list = gui_list_create(par, "", NULL, flightbook_flight_cb);

	//we want to use custom callback so we can pass another parameters
	gui_list_auto_entry(list, "Show on map", CUSTOM_CB, flightbook_flight_map_cb);

	return list;
}


