#include "flight.h"

#include "gui/tasks/menu/settings.h"

#include "take-off.h"
#include "landing.h"

#include "gui/gui_list.h"
#include "etc/format.h"

REGISTER_TASK_I(flight);



static lv_obj_t * flight_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Flight settings", &gui_settings, NULL);

	char desc[64];
	strcpy(desc, "");
	if (config_get_bool(&profile.flight.auto_take_off.alt_change_enabled))
	{
		char buff[16];
		format_altitude_with_units(buff, config_get_int(&profile.flight.auto_take_off.alt_change_value));
		sprintf(desc, "Altitude change +/-%s", buff);
	}

	if (config_get_bool(&profile.flight.auto_take_off.speed_enabled))
	{
//		char test[] = ;
		if (strlen(desc) > 0)
		{
			char or_str[] = "\n or ";
			strcat(desc, or_str);
		}

		char buff[16];
		char text[32];
		format_speed_with_units(buff, config_get_int(&profile.flight.auto_take_off.speed_value));
		sprintf(text, "Speed > %s", buff);
		strcat(desc, text);
    }

	if (strlen(desc) == 0)
		strcpy(desc, "Disabled");

	lv_obj_t * obj = gui_list_info_add_entry(list, "Automatic Take-off", desc);
	gui_config_entry_add(obj, NEXT_TASK, &gui_take_off);


	strcpy(desc, "");
	if (config_get_bool(&profile.flight.auto_landing.alt_change_enabled))
	{
		char buff[16];
		format_altitude_with_units(buff, config_get_int(&profile.flight.auto_landing.alt_change_value));
		sprintf(desc, "Altitude within +/-%s", buff);
	}

	if (config_get_bool(&profile.flight.auto_landing.speed_enabled))
	{
//		char test[] = ;
		if (strlen(desc) > 0)
		{
			char or_str[] = "\n and ";
			strcat(desc, or_str);
		}

		char buff[16];
		char text[32];
		format_speed_with_units(buff, config_get_int(&profile.flight.auto_landing.speed_value));
		sprintf(text, "Speed < %s", buff);
		strcat(desc, text);
    }

	if (strlen(desc) == 0)
		strcpy(desc, "Disabled");

	obj = gui_list_info_add_entry(list, "Automatic Landing", desc);
	gui_config_entry_add(obj, NEXT_TASK, &gui_landing);


	return list;
}



