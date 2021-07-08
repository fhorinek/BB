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
#include <gui/tasks/menu/fanet.h>
#include <gui/tasks/menu/gnss/gnss.h>
#include <gui/tasks/menu/system/system.h>
#include <gui/tasks/menu/wifi/wifi.h>
#include "gui/tasks/page/pages.h"
#include "gui/tasks/menu/flight/flight.h"

#include "gui/gui_list.h"

REGISTER_TASK_I(settings);

lv_obj_t * settings_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Device settings", &gui_pages, NULL);

	gui_config_entry_create(list, NULL, "Pilot", &gui_pilot);
	gui_config_entry_create(list, NULL, "Vario", &gui_vario);
	gui_config_entry_create(list, NULL, "Flight", &gui_flight);
	gui_config_entry_create(list, NULL, "GNSS", &gui_gnss);
	gui_config_entry_create(list, NULL, "FANET", &gui_fanet);
	gui_config_entry_create(list, NULL, "Audio", &gui_bluetooth);
	gui_config_entry_create(list, NULL, "Wi-Fi", &gui_wifi);
	gui_config_entry_create(list, NULL, "System", &gui_system);
	gui_config_entry_create(list, NULL, "Development", &gui_development);

	return list;
}
