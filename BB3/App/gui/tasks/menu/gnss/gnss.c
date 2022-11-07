#include <gui/tasks/menu/gnss/gnss.h>
#include <gui/tasks/menu/gnss/gnss_status.h>
#include <gui/tasks/menu/settings.h>
#include "../settings.h"
#include "gui/gui_list.h"

#include "fc/fc.h"
#include "etc/format.h"

#define TASK_NAME	gnss

REGISTER_TASK_IL(gnss,
	lv_obj_t * label_status;
	lv_obj_t * label_ttf;
	lv_obj_t * label_lat;
	lv_obj_t * label_lon;
);

static bool gnss_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
	if (event == LV_EVENT_CLICKED)
	{
		if (index == 0 && fc.gnss.status == fc_dev_ready)
		{
			gui_switch_task(&gui_gnss_status, LV_SCR_LOAD_ANIM_MOVE_LEFT);
		}
	}
	return true;
}


lv_obj_t * gnss_init(lv_obj_t * par)
{
    help_set_base("GNSS");

	lv_obj_t * list = gui_list_create(par, _("GNSS Settings"), &gui_settings, gnss_cb);

	local->label_status = gui_list_info_add_entry(list, _("Status"), "");
	local->label_ttf = gui_list_info_add_entry(list, _("TTF"), "");
	local->label_lat = gui_list_info_add_entry(list, _("Latitude"), "");
	local->label_lon = gui_list_info_add_entry(list, _("Longitude"), "");

	return list;
}

void gnss_loop()
{
	char sta[32];
	char ttf[16];
	char lat[32];
	char lon[32];

	if (fc.gnss.status == fc_dev_ready)
	{
	    if (fc.gnss.fix > 1)
        {
            snprintf(sta, sizeof(sta), "%uD fix %u/%u", fc.gnss.fix, fc.gnss.sat_info.sat_used, fc.gnss.sat_info.sat_total);
            snprintf(ttf, sizeof(ttf), "%0.1fs ", fc.gnss.ttf / 1000.0);

            format_gnss_datum(lat, lon, fc.gnss.latitude, fc.gnss.longtitude);
        }
        else
        {
            snprintf(sta, sizeof(sta), _("Searching %u/%u"), fc.gnss.sat_info.sat_used, fc.gnss.sat_info.sat_total);
            strcpy(ttf, _("Waiting for fix"));
            strcpy(lat, "N/A");
            strcpy(lon, "N/A");
        }
	}
	else
    {
        fc_device_status(sta, fc.gnss.status);
        strcpy(ttf, "N/A");
        strcpy(lat, "N/A");
        strcpy(lon, "N/A");
    }

	gui_list_info_set_value(local->label_status, sta);
	gui_list_info_set_value(local->label_ttf, ttf);
	gui_list_info_set_value(local->label_lat, lat);
	gui_list_info_set_value(local->label_lon, lon);
}
