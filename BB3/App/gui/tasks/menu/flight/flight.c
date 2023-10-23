#include "flight.h"

#include "gui/tasks/menu/settings.h"

#include "take-off.h"
#include "landing.h"

#include "gui/gui_list.h"
#include "etc/format.h"


REGISTER_TASK_I(flight);

static gui_list_slider_options_t dura_opt = {
	.disp_multi = 1,
	.step = 1,
	.format = format_duration,
};

static gui_list_slider_options_t scale_opt = {
        .disp_multi = 1,
        .step = 1,
        .format = format_zoom,
};

static lv_obj_t * flight_init(lv_obj_t * par)
{
    help_set_base("Flight");

	lv_obj_t * list = gui_list_create(par, _("Flight settings"), &gui_settings, NULL);

	char desc[128];
	strcpy(desc, "");
	if (config_get_bool(&profile.flight.auto_take_off.alt_change_enabled))
	{
		char buff[16];
		format_altitude_with_units(buff, config_get_int(&profile.flight.auto_take_off.alt_change_value));
		sprintf(desc, _("Altitude change +/-%s"), buff);
	}

	if (config_get_bool(&profile.flight.auto_take_off.speed_enabled))
	{
//		char test[] = ;
		if (strlen(desc) > 0)
		{
			strcat(desc, _("\n or "));
		}

		char buff[16];
		char text[32];
		format_speed_with_units(buff, config_get_int(&profile.flight.auto_take_off.speed_value));
		sprintf(text, _("Speed > %s"), buff);
		strcat(desc, text);
    }

	if (strlen(desc) == 0)
		strcpy(desc, _("Disabled"));

	lv_obj_t * obj = gui_list_info_add_entry(list, _h("Automatic Take-off"), desc);
	gui_config_entry_add(obj, NEXT_TASK, &gui_take_off);


	strcpy(desc, "");
	if (config_get_bool(&profile.flight.auto_landing.alt_change_enabled))
	{
		char buff[16];
		format_altitude_with_units(buff, config_get_int(&profile.flight.auto_landing.alt_change_value));
		sprintf(desc, _("Altitude within +/-%s"), buff);
	}

	if (config_get_bool(&profile.flight.auto_landing.speed_enabled))
	{
//		char test[] = ;
		if (strlen(desc) > 0)
		{
			strcat(desc, _("\n and "));
		}

		char buff[16];
		char text[32];
		format_speed_with_units(buff, config_get_int(&profile.flight.auto_landing.speed_value));
		sprintf(text, _("Speed < %s"), buff);
		strcat(desc, text);
    }

	if (strlen(desc) == 0)
		strcpy(desc, _("Disabled"));

	obj = gui_list_info_add_entry(list, _h("Automatic Landing"), desc);
	gui_config_entry_add(obj, NEXT_TASK, &gui_landing);


    gui_list_auto_entry(list, _h("Glide ratio time"), &profile.flight.gr_duration, &dura_opt);
    gui_list_auto_entry(list, _h("G-meter time"), &profile.flight.acc_duration, &dura_opt);
    gui_list_auto_entry(list, _h("Circling timeout"), &profile.flight.circle_timeout, &dura_opt);

    gui_list_auto_entry(list, _h("Thermal trace zoom"), &profile.map.zoom_thermal, &scale_opt);
    gui_list_auto_entry(list, _h("Use wind in trace"), &profile.flight.compensate_wind, NULL);

    gui_list_auto_entry(list, _h("Enable IGC log"), &profile.flight.logger.igc, NULL);
    gui_list_auto_entry(list, _h("Enable CSV log"), &profile.flight.logger.csv, NULL);
    gui_list_auto_entry(list, _h("Enable KML log"), &profile.flight.logger.kml, NULL);

	return list;
}



