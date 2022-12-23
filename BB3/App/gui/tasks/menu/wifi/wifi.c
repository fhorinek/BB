#include "wifi.h"
#include "wifi_list.h"
#include "wifi_info.h"
#include "../settings.h"

#include "gui/gui_list.h"
#include "fc/fc.h"
#include "drivers/esp/protocol.h"
#include "etc/format.h"

REGISTER_TASK_IL(wifi,
    lv_obj_t * network;
    lv_obj_t * info;
);

static bool wifi_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
    UNUSED(index);

	if (event == LV_EVENT_CLICKED)
	{
		if (obj == local->network)
		{
			if (fc.esp.state & ESP_STATE_WIFI_CLIENT)
				gui_switch_task(&gui_wifi_list, LV_SCR_LOAD_ANIM_MOVE_LEFT);

		}
	}

	return true;
}

static void wifi_loop()
{
	if (fc.esp.state & ESP_STATE_WIFI_CONNECTED)
    {
        char msg[32 + sizeof(fc.esp.ssid)];
        snprintf(msg, sizeof(msg), _("Connected to '%s'"), fc.esp.ssid);
        gui_list_info_set_value(local->network, msg);
    }
    else if (fc.esp.state & ESP_STATE_WIFI_CLIENT)
    {
        gui_list_info_set_value(local->network, _("Select network"));
    }
    else
    {
        gui_list_info_set_value(local->network, _("Disabled"));
    }

    char info[32];

    if (fc.esp.state & ESP_STATE_WIFI_CONNECTED && fc.esp.state & ESP_STATE_WIFI_AP)
    {
        format_ip(info, fc.esp.ip_sta);
        strcat(info, ", ");
        format_ip(info + strlen(info), fc.esp.ip_ap);
        gui_list_info_set_value(local->info, info);
    }
    else if (fc.esp.state & ESP_STATE_WIFI_CONNECTED)
    {
        format_ip(info, fc.esp.ip_sta);
        gui_list_info_set_value(local->info, info);
    }
    else if (fc.esp.state & ESP_STATE_WIFI_AP)
    {
        format_ip(info, fc.esp.ip_ap);
        gui_list_info_set_value(local->info, info);
    }
    else
    {
        gui_list_info_set_value(local->info, "");
    }
}

static lv_obj_t * wifi_init(lv_obj_t * par)
{
    help_set_base("Wifi");

	lv_obj_t * list = gui_list_create(par, _("WiFi settings"), &gui_settings, wifi_cb);

	gui_list_auto_entry(list, _h("Enable"),	&profile.wifi.enabled, NULL);
	local->network = gui_list_info_add_entry(list, _h("WiFi network"), "");
	gui_list_auto_entry(list, _h("Access point"), &profile.wifi.ap, NULL);
	gui_list_auto_entry(list, _h("Device name"), &config.device_name, NULL);
	gui_list_auto_entry(list, _h("AP password"), &config.wifi.ap_pass, NULL);
    local->info = gui_list_info_add_entry(list, _h("Network info"), "");
    gui_config_entry_add(local->info, NEXT_TASK, &gui_wifi_info);

	return list;
}



