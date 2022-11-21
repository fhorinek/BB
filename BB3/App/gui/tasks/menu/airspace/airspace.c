#include <gui/tasks/menu/airspace/display.h>
#include "gui/tasks/menu/settings.h"

#include "gui/gui_list.h"
#include "gui/tasks/filemanager.h"
#include "fc/airspaces/airspace.h"
#include "fc/fc.h"
#include "gui/tasks/page/pages.h"
#include "gui/dialog.h"

REGISTER_TASK_I(airspace);

void airspace_load_task(void * param)
{
    fc.airspaces.valid = false;
    osMutexAcquire(fc.airspaces.lock, WAIT_INF);

    uint16_t loaded;
    uint16_t hidden;
    uint32_t mem_used;

	fc.airspaces.valid = airspace_load(config_get_text(&profile.airspace.filename), true);

    gui_low_priority(false);
    dialog_close();

    for (uint8_t i = 0; i < 9; i++)
    {
        gui.map.chunks[i].ready = false;
    }

    if (!fc.airspaces.valid)
    {
        dialog_show("Error", "No airspace loaded.\n\nAre you using OpenAir format?", dialog_confirm, NULL);
        config_set_text(&profile.airspace.filename, "");
    }

    gui_switch_task(&gui_airspace, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

    osMutexRelease(fc.airspaces.lock);

    RedTaskUnregister();
    vTaskDelete(NULL);
}

bool airspace_fm_cb(uint8_t event, char * path)
{
    if (event == FM_CB_SELECT)
    {
    	char tmp[PATH_LEN];
    	filemanager_get_filename(tmp, path);

        config_set_text(&profile.airspace.filename, tmp);

    	dialog_show("Loading airspace...", tmp, dialog_progress, NULL);
    	dialog_progress_spin();

    	gui_low_priority(true);

        xTaskCreate((TaskFunction_t)airspace_load_task, "as_load_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);
        return false;
    }
	return true;
}

static bool airspace_load_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		gui_switch_task(&gui_filemanager, LV_SCR_LOAD_ANIM_MOVE_LEFT);
		filemanager_open(PATH_AIRSPACE_DIR, 0, &gui_airspace, FM_FLAG_SHOW_EXT | FM_FLAG_SORT_NAME, airspace_fm_cb);

		//supress default handler
		return false;
	}
	return true;
}

static bool airspace_unload_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
        airspace_unload();
		gui_switch_task(&gui_airspace, LV_SCR_LOAD_ANIM_NONE);

		//supress default handler
		return false;
	}
	return true;
}

static bool airspace_help_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        dialog_show("Help", "Place airspace files in OpenAir format to the airspace directory.\n\nWe recommend to use\nairspace.xcontest.org", dialog_confirm, NULL);

        //supress default handler
        return false;
    }

    return true;
}

static lv_obj_t * airspace_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Airspace settings", &gui_settings, NULL);

	if (strlen(config_get_text(&profile.airspace.filename)) == 0)
	{
		gui_list_auto_entry(list, "Load airspace", CUSTOM_CB, airspace_load_cb);
	}
	else
	{
		lv_obj_t * obj = gui_list_info_add_entry(list, "Load airspace", config_get_text(&profile.airspace.filename));
		gui_config_entry_add(obj, CUSTOM_CB, airspace_load_cb);
		gui_list_auto_entry(list, "Unload airspace", CUSTOM_CB, airspace_unload_cb);

		if (DEVEL_ACTIVE)
		{
			char tmp[64];
			snprintf(tmp, sizeof(tmp), "TBD");

			gui_list_info_add_entry(list, "Debug Info", tmp);
		}
	}

	gui_list_auto_entry(list, "Enabled classes", NEXT_TASK, &gui_airspace_display);
    gui_list_auto_entry(list, "Help", CUSTOM_CB, airspace_help_cb);

	return list;
}



