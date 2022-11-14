#include <gui/tasks/menu/development/sensors.h>
#include <gui/tasks/menu/settings.h>
#include "gui/gui_list.h"
#include "fc/fc.h"
#include "drivers/esp/esp.h"
#include "drivers/esp/protocol.h"
#include "drivers/psram.h"
#include "etc/format.h"
#include "gui/dialog.h"
#include "drivers/gnss/gnss_ublox_m8.h"
#include "gui/statusbar.h"
#include "fc/airspaces/airspace.h"

#include "gui/game/space-invaders/spaceinvaders.h"

REGISTER_TASK_IL(development,
	lv_obj_t * esp_ext_prog;
    lv_obj_t * trigger;

    uint8_t slot;
);


void development_trigger_tast(void * param)
{
	while(1)
	{
	    INFO("Copy start");
	    red_unlink("/big2");
	    copy_file("/big1", "/big2");
	    INFO("Copy end");
	}
}

void development_trigger()
{
    INFO("Development fake trigger");
    INFO("-----------------------------------------------------");

//    sound_start(PATH_TTS_DIR "/gnss_ok.wav");

 //   xTaskCreate((TaskFunction_t)development_trigger_tast, "dev_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);

    statusbar_msg_add(STATUSBAR_MSG_INFO, "development_trigger");



    //FASSERT(0);
}

static void development_loop()
{
    bool ext_active = fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual;
    bool val = gui_list_switch_get_value(local->esp_ext_prog);

    if (val != ext_active)
        gui_list_switch_set_value(local->esp_ext_prog, ext_active);
}

static bool development_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		if (obj == local->esp_ext_prog)
        {
            bool val = gui_list_switch_get_value(local->esp_ext_prog);
            bool ext_active = fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual;

            if (val != ext_active)
            {
                if (val)
                {
                    esp_enable_external_programmer(esp_external_manual);
                }
                else
                {
                    esp_disable_external_programmer();
                }
            }
        }

	}

	if (event == LV_EVENT_CLICKED)
	{
        if (obj == local->trigger)
            development_trigger();
	}

	return true;
}

static lv_obj_t * development_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Reset GNSS", &gui_settings, development_cb);

	local->slot = 0xFF;

	bool ext_active = fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual;

    local->trigger = gui_list_text_add_entry(list, "Trigger");

    gui_list_auto_entry(list, "Sensors", NEXT_TASK, &gui_sensors);

    gui_list_auto_entry(list, "ESP Disable", &config.debug.esp_off, NULL);
    gui_list_auto_entry(list, "ESP Watchdog", &config.debug.esp_wdt, NULL);
    gui_list_auto_entry(list, "Show tasks", &config.debug.tasks, NULL);
    local->esp_ext_prog = gui_list_switch_add_entry(list, "ESP ext prog", ext_active);
    gui_list_auto_entry(list, "Debug to serial", &config.debug.use_serial, NULL);
    gui_list_auto_entry(list, "Debug to USB", &config.debug.use_usb, NULL);
    gui_list_auto_entry(list, "Vario test", &config.debug.vario_test, NULL);
    gui_list_auto_entry(list, "FANET force update", &config.debug.fanet_update, NULL);
    gui_list_auto_entry(list, "Show LVGL info", &config.debug.lvgl_info, NULL);
	gui_list_auto_entry(list, "Space Invaders!", NEXT_TASK, &gui_spaceinvaders);

	return list;
}

