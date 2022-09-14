/*
 * flightbook_flight.c
 *
 * Ask the user what to do with the selected flight.
 * Show statistics, show it on map, ...
 *
 *  Created on: Feb 27, 2022
 *      Author: tilmann@bubecks.de
 */

#define DEBUG_LEVEL	DBG_DEBUG

#include "flightbook_flight.h"

#include "gui/tasks/menu/settings.h"
#include "gui/tasks/menu/flightbook/flightbook.h"
#include "gui/tasks/menu/flightbook/flightbook_flight_show.h"
// #include "gui/tasks/menu/flightbook/flightbook_flight_map.h"
#include "gui/tasks/filemanager.h"

#include "gui/statusbar.h"
#include "gui/gui_list.h"
#include "gui/ctx.h"

#include "etc/format.h"

REGISTER_TASK_I(flightbook_flight);

static bool flight_show_cb(lv_obj_t * obj, lv_event_t event)
{
	DBG("flight_show_cb");
    if (event == LV_EVENT_CLICKED)
    {
    	DBG("gui_switch_task(&gui_flightbook_flight_show");
        gui_switch_task(&gui_flightbook_flight_show, LV_SCR_LOAD_ANIM_MOVE_LEFT);
        return false;
    }
    return true;
}

static bool flight_map_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
//        gui_switch_task(&gui_flightbook_flight_map, LV_SCR_LOAD_ANIM_MOVE_LEFT);
        return false;
    }
    return true;
}

static lv_obj_t * flightbook_flight_init(lv_obj_t * par)
{
	char value[100];
	filemanager_get_filename(value, flightbook_flight_filename);

	lv_obj_t * list = gui_list_create(par, value, &gui_flightbook, NULL);

	lv_obj_t * flights = gui_list_text_add_entry(list, "Show");
	gui_config_entry_add(flights, CUSTOM_CB, flight_show_cb);

	// Additional functions, e.g. show on map
	//lv_obj_t * flights2 = gui_list_text_add_entry(list, "Analyse");
	//gui_config_entry_add(flights2, CUSTOM_CB, flight_map_cb);

	return list;
}

