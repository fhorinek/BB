
#include "gui/tasks/menu/settings.h"

#include "gui/dialog.h"
#include "gui/gui_list.h"

#include "etc/geo_calc.h"
#include "etc/format.h"

REGISTER_TASK_I(map);

static void map_cc_task(void * param)
{
    clear_dir(PATH_MAP_CACHE_DIR);
    for (uint8_t i = 0; i < 9; i++)
    {
        gui.map.chunks[i].ready = false;
    }
    gui.map.magic = (gui.map.magic + 1) % 0xFF;

    gui_low_priority(false);
    dialog_close();

    RedTaskUnregister();
    vTaskDelete(NULL);
}

static void map_cc_dialog_cb(uint8_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        dialog_show(_("Clearing map cache"), _("Please wait"), dialog_progress, NULL);
        dialog_progress_spin();
        gui_low_priority(true);

        xTaskCreate((TaskFunction_t)map_cc_task, "map_cc_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);
    }
}

static bool map_cc_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		dialog_show(_("Clear cache"), _("do you want to clear map cache?"), dialog_yes_no, map_cc_dialog_cb);

		//supress default handler
		return false;
	}

	return true;
}


static void format_zoom(char * buff, float in)
{
    int16_t zoom = config_get_int(&profile.map.zoom_flight);
    uint16_t zoom_p = pow(2, zoom);
    float guide_m = (zoom_p * 111000 * 120 / MAP_DIV_CONST);
    format_distance_with_units2(buff, guide_m);
}

static gui_list_slider_options_t scale_opt = {
    .disp_multi = 1,
    .step = 1,
    .format = format_zoom,
};


static bool map_help_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        dialog_show("Help", "To acquire maps, go to \nstrato.skybean.eu/map\n\nPlace files from *_agl.zip to agl directory.\n\nPlace files from *_map.zip to map directory.\n\nWe are also working on automatic solution.", dialog_confirm, NULL);

        //supress default handler
        return false;
    }

    return true;
}


static lv_obj_t * map_init(lv_obj_t * par)
{
    help_set_base("Map");

    lv_obj_t * list = gui_list_create(par, _("Map"), &gui_settings, NULL);

    gui_list_auto_entry(list, _("Map scale"), &profile.map.zoom_flight, &scale_opt);
    gui_list_auto_entry(list, _("Zoom to fit track"), &profile.map.zoom_fit, NULL);
    gui_list_auto_entry(list, _("Terrain type"), &profile.map.alt_range, NULL);
    gui_list_auto_entry(list, _("Topo blur"), &profile.map.blur, NULL);
    gui_list_auto_entry(list, _("Clear map cache"), CUSTOM_CB, map_cc_cb);
    gui_list_auto_entry(list, _("Show FANET on map"), &profile.map.show_fanet, NULL);
    gui_list_auto_entry(list, _("Show trail"), &profile.map.show_glider_trail, NULL);
    gui_list_auto_entry(list, _("Help"), CUSTOM_CB, map_help_cb);

    return list;
}



