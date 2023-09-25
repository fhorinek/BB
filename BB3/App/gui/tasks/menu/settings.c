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
#include "gui/tasks/menu/flightbook/flightbook.h"
#include "gui/tasks/menu/flightbook/flightbook_statistics.h"
#include "gui/tasks/menu/map.h"
#include "gui/tasks/menu/audio/audio.h"
#include "gui/tasks/menu/airspace/airspace.h"
#include "gui/tasks/menu/other.h"

#include "gui/gui_list.h"
#include "etc/format.h"
#include "fc/fc.h"

REGISTER_TASK_I(settings);

static bool open_flightbook(lv_obj_t * obj, lv_event_t event)
{
	UNUSED(obj);

	if (event == LV_EVENT_CLICKED)
	{
		flightbook_open(true);

		//supress default handler
		return false;
	}
	return true;
}

static bool stop_playback(lv_obj_t * obj, lv_event_t event)
{
	UNUSED(obj);

	if (event == LV_EVENT_CLICKED)
	{
		fc_simulate_stop();

        gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
		//supress default handler
		return false;
	}
	return true;
}

lv_obj_t * settings_init(lv_obj_t * par)
{
    help_set_base("Settings");

	lv_obj_t * list = gui_list_create(par, _("Strato settings"), &gui_pages, NULL);

	if (fc_simulate_is_playing())
		gui_list_auto_entry(list, _h("Stop Simulation"), CUSTOM_CB, stop_playback);

	gui_list_auto_entry(list, _h("Flightbook"), CUSTOM_CB, open_flightbook);
	gui_list_auto_entry(list, _h("Pilot & Flight profile"), NEXT_TASK, &gui_profiles);
	gui_list_auto_entry(list, _h("Vario"), NEXT_TASK, &gui_vario_settings);
	gui_list_auto_entry(list, _h("Flight"), NEXT_TASK, &gui_flight);
	gui_list_auto_entry(list, _h("Audio"), NEXT_TASK, &gui_audio);
	gui_list_auto_entry(list, _h("Map"), NEXT_TASK, &gui_map);
	gui_list_auto_entry(list, _h("Airspaces"), NEXT_TASK, &gui_airspace);
	gui_list_auto_entry(list, _h("FANET"), NEXT_TASK, &gui_fanet);
	gui_list_auto_entry(list, _h("GNSS"), NEXT_TASK, &gui_gnss);
	gui_list_auto_entry(list, _h("Bluetooth"), NEXT_TASK, &gui_bluetooth);
	gui_list_auto_entry(list, _h("Wi-Fi"), NEXT_TASK, &gui_wifi);
	gui_list_auto_entry(list, _h("System"), NEXT_TASK, &gui_system);
	gui_list_auto_entry(list, _h("Other"), NEXT_TASK, &gui_other);

	if (DEVEL_ACTIVE)
		gui_list_auto_entry(list, _h("Development"), NEXT_TASK, &gui_development);

	return list;
}

