#include "auto_poweroff.h"
#include "system.h"

#include "gui/gui_list.h"

#include "fc/fc.h"
#include "etc/format.h"


REGISTER_TASK_I(auto_poweroff);

static void format_min(char * buff, float in)
{
    uint16_t val = in;
    sprintf(buff, "%u min", val);
}

static gui_list_slider_options_t min_param =
{
	.disp_multi	= 1,
	.step = 1,
	.format = format_min
};

lv_obj_t * auto_poweroff_init(lv_obj_t * par)
{
    help_set_base("System/Auto_poweroff");

	lv_obj_t * list = gui_list_create(par, _("Auto power off"), &gui_system, NULL);

	gui_list_note_add_entry(list, _h("When idle, power off the device"), LV_COLOR_BLACK);
    gui_list_auto_entry(list, _h("Enabled"), &profile.ui.auto_power_off_enabled, NULL);
    gui_list_auto_entry(list, _h("Timeout"), &profile.ui.auto_power_off_time, &min_param);

    gui_list_note_add_entry(list, _h("When flying, return to main screen after 30 seconds"), LV_COLOR_BLACK);
    gui_list_auto_entry(list, _h("Enabled"), &profile.ui.return_to_pages, NULL);

	return list;
}

