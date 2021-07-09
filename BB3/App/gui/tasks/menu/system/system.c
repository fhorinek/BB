/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */



#include "gui/gui_list.h"

#include "datetime/datetime.h"
#include "calibration.h"
#include "display.h"
#include "info.h"
#include "../settings.h"

#include "drivers/rev.h"

REGISTER_TASK_I(system,
	lv_obj_t * info;
);

static bool system_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
    if (event == LV_EVENT_CLICKED)
    {
        if (obj == local->info)
            gui_switch_task(&gui_info, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    }

    return true;
}


lv_obj_t * system_init(lv_obj_t * par)
{
    DBG("settings init");

    lv_obj_t * list = gui_list_create(par, "System", &gui_settings, system_cb);

    gui_list_auto_entry(list, "Time & date", NEXT_TASK, &gui_datetime);
    gui_list_auto_entry(list, "Display", NEXT_TASK, &gui_display);
    gui_list_auto_entry(list, "Calibration", NEXT_TASK, &gui_calibration);

    char rev_str[10];
    rew_get_sw_string(rev_str);
    local->info = gui_list_info_add_entry(list, "Device info", rev_str);

    return list;
}
