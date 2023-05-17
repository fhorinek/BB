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
    lv_obj_t * vario_fake;

    uint8_t slot;
);


void development_trigger_task(void * param)
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

 //   xTaskCreate((TaskFunction_t)development_trigger_task, "dev_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);

    statusbar_msg_add(STATUSBAR_MSG_INFO, "development_trigger");

    uint8_t * a = tmalloc(8);
    uint8_t * b = tmalloc(8);
    memset(a, 0, 16);
    //tfree(a);
    tfree(b);

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

		if (obj == local->vario_fake)
		{
            fc.fused.fake = true;
            fc.fused.vario = gui_list_slider_get_value(obj) / 10.0;
		}

	}

	if (event == LV_EVENT_CLICKED)
	{
        if (obj == local->trigger)
            development_trigger();
	}

	return true;
}

static bool esp_reset_nvm_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        protocol_send(PROTO_RESET_NVM, NULL, 0);
    }

    return true;
}

static lv_obj_t * development_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Reset GNSS", &gui_settings, development_cb);

	local->slot = 0xFF;

	bool ext_active = fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual;

    local->trigger = gui_list_text_add_entry(list, "Trigger", 0);

    gui_list_auto_entry(list, "Sensors", 0, NEXT_TASK, &gui_sensors);

    gui_list_auto_entry(list, "STM idle sleep", 0, &config.system.enable_sleep, NULL);
    gui_list_auto_entry(list, "ESP Disable", 0, &config.debug.esp_off, NULL);
    gui_list_auto_entry(list, "ESP Watchdog", 0, &config.debug.esp_wdt, NULL);
    gui_list_auto_entry(list, "ESP enter gdbstub", 0, &config.debug.esp_gdbstub, NULL);
    gui_list_auto_entry(list, "ESP Reset NVM", 0, CUSTOM_CB, esp_reset_nvm_cb);
    gui_list_auto_entry(list, "Show tasks", 0, &config.debug.tasks, NULL);
    local->esp_ext_prog = gui_list_switch_add_entry(list, "ESP ext prog", 0, ext_active);
    gui_list_auto_entry(list, "Add help entries", 0, &config.debug.help_show_id, NULL);
    gui_list_auto_entry(list, "Debug to serial", 0, &config.debug.use_serial, NULL);
    gui_list_auto_entry(list, "Debug to USB", 0, &config.debug.use_usb, NULL);

    gui_list_auto_entry(list, "Vario test", 0, &config.debug.vario_test, NULL);
    local->vario_fake = gui_list_slider_add_entry(list, "Fake vario value", 0, -10, 10, 0);

    gui_list_auto_entry(list, "FANET force update", 0, &config.debug.fanet_update, NULL);
    gui_list_auto_entry(list, "Show LVGL info", 0, &config.debug.lvgl_info, NULL);
	gui_list_auto_entry(list, "Space Invaders!", 0, NEXT_TASK, &gui_spaceinvaders);

	return list;
}

