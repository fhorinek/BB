#include <gui/tasks/menu/development/sensors.h>
#include <gui/tasks/menu/fanet.h>
#include <gui/tasks/menu/settings.h>
#include "settings.h"
#include "gui/gui_list.h"
#include "fc/fc.h"
#include "drivers/esp/esp.h"

REGISTER_TASK_I(bluetooth,
	lv_obj_t * volume;
);

static void bluetooth_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

	if (event == LV_EVENT_VALUE_CHANGED)
	{
		switch(index)
		{
			case 0:
			{
			    uint8_t vol = gui_list_slider_get_value(local->volume);
			    esp_set_volume(vol);
			    config_set_int(&config.bluetooth.volume, vol);
            }
            break;
		}


	}

}



static lv_obj_t * bluetooth_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Bluetooth", bluetooth_cb);

    local->volume = gui_list_slider_add_entry(list, "Volume", 0, 100, config_get_int(&config.bluetooth.volume));

	return list;
}



