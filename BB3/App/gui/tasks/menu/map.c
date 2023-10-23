#include "gui/tasks/menu/settings.h"

#include "gui/dialog.h"
#include "gui/gui_list.h"

#include "etc/geo_calc.h"
#include "etc/format.h"
#include "fc/fc.h"

REGISTER_TASK_IL(map,
        lv_obj_t *map_info;
);

static void map_cc_task(void *param)
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

static void map_cc_dialog_cb(uint8_t res, void *data)
{
    if (res == dialog_res_yes)
    {
        dialog_show(_("Clearing map cache"), _("Please wait"), dialog_progress, NULL);
        dialog_progress_spin();
        gui_low_priority(true);

        xTaskCreate((TaskFunction_t) map_cc_task, "map_cc_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);
    }
}

static bool map_cc_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        dialog_show(_("Clear cache"), _("do you want to clear map cache?"), dialog_yes_no, map_cc_dialog_cb);

        //supress default handler
        return false;
    }

    return true;
}

static gui_list_slider_options_t scale_opt = {
        .disp_multi = 1,
        .step = 1,
        .format = format_zoom,
};

static bool map_help_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        dialog_show(_("Help"), _("To acquire maps, go to \nstrato.skybean.eu/map\n\nPlace files from *_agl.zip to agl directory.\n\nPlace files from *_map.zip to map directory.\n\nWe are also working on automatic solution."), dialog_confirm, NULL);

        //supress default handler
        return false;
    }

    return true;
}

static bool map_index_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        map_index_show();

        return false;
    }

    return true;
}

static lv_obj_t* map_init(lv_obj_t *par)
{
    help_set_base("Map");

    lv_obj_t *list = gui_list_create(par, _("Map"), &gui_settings, NULL);

    local->map_info = gui_list_info_add_entry(list, _h("Map management"), "\n");
    gui_config_entry_add(local->map_info, CUSTOM_CB, map_index_cb);

    gui_list_auto_entry(list, _h("Map scale"), &profile.map.zoom_flight, &scale_opt);
    gui_list_auto_entry(list, _h("Zoom to fit track"), &profile.map.zoom_fit, NULL);
    gui_list_auto_entry(list, _h("Terrain type"), &profile.map.alt_range, NULL);
    gui_list_auto_entry(list, _h("Topo blur"), &profile.map.blur, NULL);
    gui_list_auto_entry(list, _h("Clear map cache"), CUSTOM_CB, map_cc_cb);
    gui_list_auto_entry(list, _h("Show FANET on map"), &profile.map.show_fanet, NULL);
    gui_list_auto_entry(list, _h("Show trail"), &profile.map.show_glider_trail, NULL);
    gui_list_auto_entry(list, _h("Help"), CUSTOM_CB, map_help_cb);

    return list;
}

static void map_loop()
{
    char tmp[64];

    switch (fc.map_index.status)
    {
        case (MAP_INDEX_OK):
            snprintf(tmp, sizeof(tmp), _("Select countries\nTiles: %u/%u (%u failed)"), fc.map_index.tiles_index, fc.map_index.tiles_requested, fc.map_index.tiles_failed);
        break;

        case (MAP_INDEX_INDEXING):
            snprintf(tmp, sizeof(tmp), _("Indexing...\nTiles done: %u"), fc.map_index.tiles_requested);
        break;

        case (MAP_INDEX_DOWNLOADING_AGL):
            snprintf(tmp, sizeof(tmp), _("Getting AGL %s\n%u%%Tiles done: %u/%u"), fc.map_index.tile, fc.map_index.process, fc.map_index.tiles_index, fc.map_index.tiles_requested);
        break;

        case (MAP_INDEX_DOWNLOADING_MAP):
            snprintf(tmp, sizeof(tmp), _("Getting MAP %s\n%u%% Tiles done: %u/%u"), fc.map_index.tile, fc.map_index.process, fc.map_index.tiles_index, fc.map_index.tiles_requested);
        break;

        case (MAP_INDEX_UNZIPPING_AGL):
            snprintf(tmp, sizeof(tmp), _("Extracting AGL %s\nTiles done: %u/%u"), fc.map_index.tile, fc.map_index.tiles_index, fc.map_index.tiles_requested);
        break;

        case (MAP_INDEX_UNZIPPING_MAP):
            snprintf(tmp, sizeof(tmp), _("Extracting MAP %s\nTiles done: %u/%u"), fc.map_index.tile, fc.map_index.tiles_index, fc.map_index.tiles_requested);
        break;

    }

    gui_list_info_set_value(local->map_info, tmp);
}

