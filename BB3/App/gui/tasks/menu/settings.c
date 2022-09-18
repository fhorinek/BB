/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */

#include <gui/tasks/menu/settings.h>

#include <gui/tasks/menu/profiles/profiles.h>
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
#include "gui/tasks/menu/audio.h"
#include "gui/game/space-invaders/spaceinvaders.h"

#include "gui/gui_list.h"
#include "etc/format.h"

REGISTER_TASK_I(settings);

static bool spaceinvaders_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		gui_switch_task(&gui_spaceinvaders, LV_SCR_LOAD_ANIM_MOVE_LEFT);
	}

	return true;
}

lv_obj_t * settings_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Strato settings", &gui_pages, NULL);

	gui_list_auto_entry(list, "Pilot & Flight profile", NEXT_TASK, &gui_profiles);
	gui_list_auto_entry(list, "Vario", NEXT_TASK, &gui_vario_settings);
	gui_list_auto_entry(list, "Flight", NEXT_TASK, &gui_flight);
	gui_list_auto_entry(list, "Audio", NEXT_TASK, &gui_audio);
	gui_list_auto_entry(list, "Map", NEXT_TASK, &gui_map);
	gui_list_auto_entry(list, "FANET", NEXT_TASK, &gui_fanet);
	gui_list_auto_entry(list, "GNSS", NEXT_TASK, &gui_gnss);
	gui_list_auto_entry(list, "Bluetooth", NEXT_TASK, &gui_bluetooth);
	gui_list_auto_entry(list, "Wi-Fi", NEXT_TASK, &gui_wifi);
	gui_list_auto_entry(list, "Space Invaders!", CUSTOM_CB, &spaceinvaders_cb);
	gui_list_auto_entry(list, "System", NEXT_TASK, &gui_system);

	if (DEVEL_ACTIVE)
		gui_list_auto_entry(list, "Development", NEXT_TASK, &gui_development);

	return list;
}

