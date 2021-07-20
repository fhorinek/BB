#include "display.h"
#include "system.h"

#include "gui/gui_list.h"

#include "drivers/power/led.h"

#include "fc/fc.h"
#include "etc/format.h"

REGISTER_TASK_I(display);

static gui_list_slider_options_t back_param =
{
	.disp_multi	= 1,
	.step = 10,
	.format = format_percent
};

lv_obj_t * display_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Display Settings", &gui_system, NULL);

	gui_list_auto_entry(list, "Brightness", &config.display.backlight, &back_param);
	gui_list_auto_entry(list, "Battery percent", &config.display.bat_per, NULL);
	gui_list_auto_entry(list, "Welcome message", &config.display.show_msg, NULL);

	return list;
}

