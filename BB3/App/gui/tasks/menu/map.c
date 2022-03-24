
#include "gui/tasks/menu/settings.h"

#include "gui/dialog.h"
#include "gui/gui_list.h"

REGISTER_TASK_I(map);


void map_cc_dialog_cb(uint8_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        clear_dir(PATH_MAP_CACHE_DIR);
        for (uint8_t i = 0; i < 9; i++)
        {
            gui.map.chunks[i].ready = false;
        }
        gui.map.magic = (gui.map.magic + 1) % 0xFF;
    }
}

static bool map_cc_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		dialog_show("Clear cache", "do you want to clear map cache?", dialog_yes_no, map_cc_dialog_cb);

		//supress default handler
		return false;
	}

	return true;
}



static lv_obj_t * map_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Map", &gui_settings, NULL);

    gui_list_auto_entry(list, "Zoom", &profile.map.zoom_flight, NULL);
    gui_list_auto_entry(list, "Terrain type", &profile.map.alt_range, NULL);
    gui_list_auto_entry(list, "Topo blur", &profile.map.blur, NULL);
    gui_list_auto_entry(list, "Clear cache", CUSTOM_CB, map_cc_cb);
    gui_list_auto_entry(list, "Show FANET on map", &profile.map.show_fanet, NULL);

    return list;
}



