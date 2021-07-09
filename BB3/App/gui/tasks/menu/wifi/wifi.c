#include <gui/tasks/menu/settings.h>
#include <gui/tasks/menu/wifi/wifi.h>
#include <gui/tasks/menu/wifi/wifi_list.h>
#include <gui/tasks/menu/wifi/wifi_info.h>
#include "../settings.h"

#include "gui/gui_list.h"
#include "fc/fc.h"
#include "drivers/esp/protocol.h"

REGISTER_TASK_IL(wifi,
    lv_obj_t * en_sta;
    lv_obj_t * en_ap;
    lv_obj_t * dev_name;
    lv_obj_t * ap_pass;
    lv_obj_t * network;
);

static bool wifi_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CLICKED)
	{
		switch(index)
		{
            case 1:
                if (fc.esp.state & ESP_STATE_WIFI_CLIENT)
                    gui_switch_task(&gui_wifi_list, LV_SCR_LOAD_ANIM_MOVE_LEFT);
            break;

            case 5:
                gui_switch_task(&gui_wifi_info, LV_SCR_LOAD_ANIM_MOVE_LEFT);
            break;
		}


	}

	if (event == LV_EVENT_VALUE_CHANGED)
	{
        if (obj == local->en_sta)
        {
            config_set_bool(&config.wifi.enabled, gui_list_switch_get_value(obj));
            esp_set_wifi_mode();
        }

        if (obj == local->en_ap)
        {
            config_set_bool(&config.wifi.ap, gui_list_switch_get_value(obj));
            esp_set_wifi_mode();
        }

	    if (obj == local->dev_name)
	    {
	        config_set_text(&config.device_name, gui_list_textbox_get_value(obj));
	    }
        if (obj == local->ap_pass)
        {
            config_set_text(&config.wifi.ap, gui_list_textbox_get_value(obj));
        }

	}
	return false;
}

static void wifi_loop()
{
    if (fc.esp.state & ESP_STATE_WIFI_CONNECTED)
    {
        char msg[16 + sizeof(fc.esp.ssid)];
        snprintf(msg, sizeof(msg), "Connected to '%s'", fc.esp.ssid);
        gui_list_info_set_value(local->network, msg);
    }
    else if (fc.esp.state & ESP_STATE_WIFI_CLIENT)
    {
        gui_list_info_set_value(local->network, "Select network");
    }
    else
        gui_list_info_set_value(local->network, "Disabled");
}

static lv_obj_t * wifi_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "WiFi settings", &gui_settings, wifi_cb);

    local->en_sta = gui_list_switch_add_entry(list, "Enable", config_get_bool(&config.wifi.enabled));
    local->network = gui_list_info_add_entry(list, "WiFi network", "");
    local->en_ap = gui_list_switch_add_entry(list, "Access point", config_get_bool(&config.wifi.ap));
    local->dev_name = gui_list_textbox_add_entry(list, "Device name", config_get_text(&config.device_name), DEV_NAME_LEN);
    local->ap_pass = gui_list_textbox_add_entry(list, "AP password", config_get_text(&config.wifi.ap_pass), WIFI_PASS_LEN);
    gui_list_text_add_entry(list, "Network info");

	return list;
}



