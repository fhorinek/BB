#include <gui/tasks/menu/airspace/display.h>
#include "gui/tasks/menu/settings.h"

#include "gui/gui_list.h"
#include "gui/tasks/filemanager.h"
#include "fc/airspaces/airspace.h"
#include "fc/fc.h"
#include "gui/tasks/page/pages.h"
#include "gui/dialog.h"

#include "etc/geo_calc.h"

REGISTER_TASK_IL(airspace,
    lv_obj_t * dbg_info;
);

void airspace_load_task(void * param)
{
    osMutexAcquire(fc.airspaces.lock, WAIT_INF);

	bool ret = airspace_load(config_get_text(&profile.airspace.filename), true);

    gui_low_priority(false);
    dialog_close();

    if (!ret)
    {
        dialog_show(_("Error"), _("No airspace loaded.\n\nAre you using OpenAir format?"), dialog_confirm, NULL);
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

    	dialog_show(_("Loading airspace..."), tmp, dialog_progress, NULL);
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
        config_set_text(&profile.airspace.filename, "");
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
        dialog_show(_("Help"), _("Place airspace files in OpenAir format to the airspace directory.\n\nWe recommend to use\nairspace.xcontest.org"), dialog_confirm, NULL);

        //supress default handler
        return false;
    }

    return true;
}

static void airspace_loop()
{
    if (local->dbg_info != NULL)
    {
        float dist = geo_distance(gui.map.lat, gui.map.lon, fc.airspaces.valid_lat, fc.airspaces.valid_lon, true, NULL) / 100000.0;

        char tmp[128];
        snprintf(tmp, sizeof(tmp), _("Loaded %lu/%u\nData used %lu%%\nIn file %lu\nDist %0.1fkm"),
                fc.airspaces.number_loaded, AIRSPACE_INDEX_ALLOC,
                (fc.airspaces.data_used * 100) / AIRSPACE_DATA_ALLOC,
                fc.airspaces.number_in_file,
                dist);

        gui_list_info_set_value(local->dbg_info, tmp);
    }
}

static lv_obj_t * airspace_init(lv_obj_t * par)
{
    help_set_base("Airspace");

    local->dbg_info = NULL;

	lv_obj_t * list = gui_list_create(par, _("Airspace settings"), &gui_settings, NULL);

	if (strlen(config_get_text(&profile.airspace.filename)) == 0)
	{
		gui_list_auto_entry(list, _h("Load airspace"), CUSTOM_CB, airspace_load_cb);
	}
	else
	{
		lv_obj_t * obj = gui_list_info_add_entry(list, _h("Load airspace"), config_get_text(&profile.airspace.filename));
		gui_config_entry_add(obj, CUSTOM_CB, airspace_load_cb);
		gui_list_auto_entry(list, _h("Unload airspace"), CUSTOM_CB, airspace_unload_cb);

		if (DEVEL_ACTIVE)
		{
			local->dbg_info = gui_list_info_add_entry(list, _h("Debug Info"), "-\n-\n-\n");
		}
	}

	gui_list_auto_entry(list, _h("Enabled classes"), NEXT_TASK, &gui_airspace_display);
	gui_list_auto_entry(list, _h("Help"), CUSTOM_CB, airspace_help_cb);

	return list;
}



