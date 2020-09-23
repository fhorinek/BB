#include "gnss.h"
#include "settings.h"
#include "gnss_status.h"

#include "../gui_list.h"

#include "../../config/config.h"
#include "../../fc/fc.h"
#include "../../etc/format.h"

#define TASK_NAME	gnss

REGISTER_TASK_IL(gnss,
	lv_obj_t * label_status;
	lv_obj_t * label_ttf;
	lv_obj_t * label_lat;
	lv_obj_t * label_lon;
);

void gnss_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

	if (event == LV_EVENT_CLICKED)
	{
		if (index == 1)
			gui_switch_task(&gui_gnss_status, LV_SCR_LOAD_ANIM_MOVE_LEFT);
	}

}


lv_obj_t * gnss_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "GNSS Settings", gnss_cb);

	local->label_status = gui_list_info_add_entry(list, "Status", "");
	local->label_ttf = gui_list_info_add_entry(list, "TTF", "");
	local->label_lat = gui_list_info_add_entry(list, "Latitude", "");
	local->label_lon = gui_list_info_add_entry(list, "Longitude", "");

	return list;
}

void gnss_loop()
{
	char sta[32];
	char ttf[16];
	char lat[16];
	char lon[16];

	if (fc.gnss.valid)
	{
		snprintf(sta, sizeof(sta), "%uD fix %u/%u", fc.gnss.fix, fc.gnss.sat_info.sat_used, fc.gnss.sat_info.sat_total);
		snprintf(ttf, sizeof(ttf), "%0.1fs ", fc.gnss.ttf / 1000.0);

		format_gnss_datum(lat, lon, fc.gnss.latitude, fc.gnss.longtitude);
	}
	else
	{
		snprintf(sta, sizeof(sta), "Searching %u/%u", fc.gnss.sat_info.sat_used, fc.gnss.sat_info.sat_total);
		strcpy(ttf, "Waiting for fix");
		strcpy(lat, "N/A");
		strcpy(lon, "N/A");
	}

	gui_list_info_set_value(local->label_status, sta);
	gui_list_info_set_value(local->label_ttf, ttf);
	gui_list_info_set_value(local->label_lat, lat);
	gui_list_info_set_value(local->label_lon, lon);
}
