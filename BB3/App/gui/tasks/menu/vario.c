#include "vario.h"

#include "gui/tasks/menu/settings.h"

#include "gui/gui_list.h"
#include "gui/tasks/filemanager.h"

#include "etc/format.h"
#include "fc/vario.h"

REGISTER_TASK_I(vario_settings);

static gui_list_slider_options_t sink_lift_opt = {
	.disp_multi = 0.1,
	.step = 1,
	.format = format_vario_with_units,
};

static gui_list_slider_options_t acc_opt = {
	.disp_multi = 100,
	.step = 0.1,
	.format = format_percent,
};

static gui_list_slider_options_t avg_opt = {
	.disp_multi = 1,
	.step = 1,
	.format = format_duration,
};

bool vario_profile_fm_cb(uint8_t event, char * path)
{
    if (event == FM_CB_SELECT)
    {
        path = strrchr(path, '/');

        if (path == NULL)
            return true;
        path++;
        char * dot = strrchr(path, '.');

        if (dot == NULL)
            return true;
        *dot = 0;

        config_set_text(&profile.vario.profile, path);
        vario_profile_load(path);
    }
	return true;
}

static bool vario_profile_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		gui_switch_task(&gui_filemanager, LV_SCR_LOAD_ANIM_MOVE_LEFT);
		filemanager_open(PATH_VARIO_DIR, 0, &gui_vario_settings, FM_FLAG_HIDE_DIR | FM_FLAG_SORT_NAME, vario_profile_fm_cb);

		//supress default handler
		return false;
	}
	return true;
}

static lv_obj_t * vario_settings_init(lv_obj_t * par)
{
    help_set_base("Vario");

    lv_obj_t * list = gui_list_create(par, _("Vario settings"), &gui_settings, NULL);

	gui_list_auto_entry(list, _h("Audio only in flight"), &profile.vario.in_flight, NULL);
	gui_list_auto_entry(list, _h("Accelerometer gain"), &profile.vario.acc_gain, &acc_opt);
	gui_list_auto_entry(list, _h("Lift threshold"), &profile.vario.lift, &sink_lift_opt);
	gui_list_auto_entry(list, _h("Sink threshold"), &profile.vario.sink, &sink_lift_opt);
	gui_list_auto_entry(list, _h("Average time"), &profile.vario.avg_duration, &avg_opt);

	lv_obj_t * obj = gui_list_info_add_entry(list, _h("Vario profile"), config_get_text(&profile.vario.profile));
	gui_config_entry_add(obj, CUSTOM_CB, vario_profile_cb);

	return list;
}



