#include "fanet.h"

#include "settings.h"
#include "sensors.h"

#include "../gui_list.h"
#include "../../fc/fc.h"
#include "../../drivers/esp/esp.h"

REGISTER_TASK_I(development,
	lv_obj_t * esp_ext_prog;
    lv_obj_t * usb_otg_pin;
);

static void development_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

	if (event == LV_EVENT_VALUE_CHANGED)
	{
		switch(index)
		{
			case 0:
			{
				bool val = gui_list_switch_get_value(local->esp_ext_prog);
				if (val)
				{
					esp_enable_external_programmer();
				}
				else
				{
					esp_disable_external_programmer();
				}
			}
			break;

            case 1:
            {
                bool val = gui_list_switch_get_value(local->usb_otg_pin);
                GpioWrite(CH_EN_OTG, !val);
            }
            break;
		}           //power up the negotiator


	}

	if (event == LV_EVENT_CLICKED)
	{
		if (index == 2)
			gui_switch_task(&gui_sensors, LV_SCR_LOAD_ANIM_MOVE_LEFT);
	}
}



static lv_obj_t * development_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Develpment", development_cb);

    local->esp_ext_prog = gui_list_switch_add_entry(list, "ESP ext prog", fc.esp.mode == esp_external);
    local->usb_otg_pin = gui_list_switch_add_entry(list, "USB OTG pin", GpioRead(CH_EN_OTG));
	gui_list_text_add_entry(list, "Sensors");

	return list;
}



