#include "display.h"
#include "system.h"

#include "gui/gui_list.h"

#include "drivers/power/led.h"

#include "fc/fc.h"
#include "etc/format.h"
#include "drivers/power/pwr_mng.h"

REGISTER_TASK_I(display,
    uint8_t old_lang;
);

static gui_list_slider_options_t back_param =
{
	.disp_multi	= 1,
	.step = 10,
	.format = format_percent
};

static bool display_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
    if (local->old_lang != config_get_select(&config.display.language))
    {
        gui_switch_task(&gui_display, LV_SCR_LOAD_ANIM_NONE);
        return false;
    }

    return true;
}

lv_obj_t * display_init(lv_obj_t * par)
{
    help_set_base("System/Display");

    local->old_lang = config_get_select(&config.display.language);

	lv_obj_t * list = gui_list_create(par, _("Display Settings"), &gui_system, display_cb);

	gui_list_auto_entry(list, _("Language"), &config.display.language, NULL);
	gui_list_auto_entry(list, _("Brightness"), &config.display.backlight, &back_param);
	gui_list_auto_entry(list, _("Page animation"), &config.display.page_anim, NULL);

	if (pwr.fuel_gauge.status == fc_dev_ready)
		gui_list_auto_entry(list, _("Battery percent"), &config.display.bat_per, NULL);

	return list;
}

