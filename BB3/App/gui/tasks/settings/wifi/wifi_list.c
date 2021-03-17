#include "wifi_list.h"

#include "wifi.h"

#include "gui/gui_list.h"
#include "drivers/esp/esp.h"
#include "drivers/esp/protocol.h"


#include "config/config.h"

REGISTER_TASK_IS(wifi_list,
    lv_obj_t * spinner;
    uint8_t magic;
);

static void wifi_list_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_wifi, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

	if (event == LV_EVENT_VALUE_CHANGED)
	{
		switch(index)
		{
		}


	}

}

void wifi_list_update(proto_wifi_scan_res_t * network)
{
    if (local->magic != network->magic)
    {
        local->magic = network->magic;

        for (uint8_t i = 0; ; i++)
        {
            lv_obj_t * item = gui_list_get_entry(i);
            if (item == NULL)
                break;
            lv_obj_del_async(item);
        }

        lv_obj_set_hidden(local->spinner, true);
    }

    gui_list_info_add_entry(gui.list.object, network->name, "mac;security;ssi");
}


static lv_obj_t * wifi_list_init(lv_obj_t * par)
{
    local->magic = 0;
    esp_wifi_start_scan(wifi_list_update);

	lv_obj_t * list = gui_list_create(par, "Select network", wifi_list_cb);

    local->spinner = lv_spinner_create(par, NULL);
    lv_obj_set_size(local->spinner, 200, 200);
    lv_obj_align(local->spinner, NULL, LV_ALIGN_CENTER, 0, 0);

	return list;
}

static void wifi_list_stop()
{
    esp_wifi_stop_scan();
}
