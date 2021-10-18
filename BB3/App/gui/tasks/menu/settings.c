/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */

#include <gui/tasks/menu/settings.h>

#include <gui/tasks/menu/pilot.h>
#include <gui/tasks/menu/vario.h>
#include <gui/tasks/menu/bluetooth.h>
#include <gui/tasks/menu/development/development.h>
#include <gui/tasks/menu/fanet/fanet.h>
#include <gui/tasks/menu/gnss/gnss.h>
#include <gui/tasks/menu/system/system.h>
#include <gui/tasks/menu/wifi/wifi.h>
#include "gui/tasks/page/pages.h"
#include "gui/tasks/menu/flight/flight.h"
#include "gui/tasks/menu/map.h"

#include "gui/gui_list.h"

REGISTER_TASK_IS(settings);

lv_obj_t * settings_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Device settings", &gui_pages, NULL);

	gui_list_auto_entry(list, "Pilot", NEXT_TASK, &gui_pilot);
	gui_list_auto_entry(list, "Vario", NEXT_TASK, &gui_vario_settings);
	gui_list_auto_entry(list, "Flight", NEXT_TASK, &gui_flight);
//	gui_list_auto_entry(list, "Map", NEXT_TASK, &gui_map);
	gui_list_auto_entry(list, "FANET", NEXT_TASK, &gui_fanet);
	gui_list_auto_entry(list, "GNSS", NEXT_TASK, &gui_gnss);
	gui_list_auto_entry(list, "Bluetooth", NEXT_TASK, &gui_bluetooth);
	gui_list_auto_entry(list, "Wi-Fi", NEXT_TASK, &gui_wifi);
	gui_list_auto_entry(list, "System", NEXT_TASK, &gui_system);

	if (file_exists(DEV_MODE_FILE))
		gui_list_auto_entry(list, "Development", NEXT_TASK, &gui_development);

	return list;
}

void settings_stop()
{
	if (gui.task.actual == &gui_pages)
	{
		config_store_all();
	}
}
