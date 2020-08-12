#include "gnss.h"
#include "settings.h"
#include "gnss_status.h"

#include "../gui_list.h"

#include "../../config/config.h"
#include "../../fc/fc.h"
#include "../../etc/format.h"

typedef struct
{
	lv_obj_t * gnss_sw;
	lv_obj_t * label_status;
	lv_obj_t * label_lat;
	lv_obj_t * label_lon;

} local_vars_t;

void gnss_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, GUI_SW_LEFT_RIGHT);

	if (event == LV_EVENT_VALUE_CHANGED)
	{
		switch(index)
		{
			case 0:
			{
				bool val = gui_list_switch_get_value(local.gnss_sw);
				config_set_bool(&config.devices.gnss.enabled, val);
			}
			break;
		}

	}

	if (event == LV_EVENT_CLICKED)
	{
		if (index == 1)
			gui_switch_task(&gui_gnss_status, GUI_SW_RIGHT_LEFT);
	}

}


lv_obj_t * gnss_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "GNSS Settings", gnss_cb);

	local.gnss_sw = gui_list_switch_add_entry(list, "Enable GNSS", config_get_bool(&config.devices.gnss.enabled));

	local.label_status = gui_list_info_add_entry(list, "Status", "");
	local.label_lat = gui_list_info_add_entry(list, "Latitude", "");
	local.label_lon = gui_list_info_add_entry(list, "Longitude", "");

	return list;
}

void gnss_loop()
{
	char fix[32];
	char lat[16];
	char lon[16];

	uint32_t ttf = fc.gnss.fix_time;
	if (fc.gnss.first_fix)
		ttf = HAL_GetTick() - fc.gnss.fix_time;

	sprintf(fix, "TTF: %0.1fs ", ttf / 1000.0);

	if (fc.gnss.valid)
	{
		sprintf(fix + strlen(fix), "%uD fix",  fc.gnss.fix);

		format_gnss_datum(lat, lon, fc.gnss.latitude, fc.gnss.longtitude);
	}
	else
	{
		if (config_get_bool(&config.devices.gnss.enabled))
			strcpy(fix + strlen(fix), "Waiting for fix");
		else
			strcpy(fix, "Disabled");

		strcpy(lat, "N/A");
		strcpy(lon, "N/A");
	}

	gui_list_info_set_value(local.label_status, fix);
	gui_list_info_set_value(local.label_lat, lat);
	gui_list_info_set_value(local.label_lon, lon);
}

bool gnss_stop()
{
	return true;
}

gui_task_t gui_gnss =
{
	gnss_init,
	gnss_loop,
	gnss_stop
};
