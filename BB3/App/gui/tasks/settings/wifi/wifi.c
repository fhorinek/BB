#include "wifi.h"
#include "wifi_list.h"

#include "../settings.h"

#include "gui/gui_list.h"
#include "fc/fc.h"
#include "drivers/esp/esp.h"

#include "config/config.h"

REGISTER_TASK_I(wifi,
    lv_obj_t * en_sta;
    lv_obj_t * en_ap;
);

static void wifi_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

	if (event == LV_EVENT_CLICKED)
	{
		switch(index)
		{
			case 1:
			{
			    gui_switch_task(&gui_wifi_list, LV_SCR_LOAD_ANIM_MOVE_LEFT);
			}
            break;
		}


	}

}



static lv_obj_t * wifi_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "WiFi settings", wifi_cb);

    local->en_sta = gui_list_switch_add_entry(list, "Enable", config_get_bool(&config.wifi.enabled));
    gui_list_info_add_entry(list, "Select network", "");
    local->en_ap = gui_list_switch_add_entry(list, "Access point", config_get_bool(&config.wifi.ap));
    gui_list_info_add_entry(list, "AP name", config_get_text(&config.device_name));

	return list;
}



