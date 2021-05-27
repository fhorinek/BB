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

REGISTER_TASK_I(system);

void system_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
    if (event == LV_EVENT_CANCEL)
        gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

    if (event == LV_EVENT_CLICKED)
    {
        if (index == 0)
            gui_switch_task(&gui_datetime, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 1)
            gui_switch_task(&gui_display, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 2)
            gui_switch_task(&gui_calibration, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 3)
            gui_switch_task(&gui_info, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    }

}


lv_obj_t * system_init(lv_obj_t * par)
{
    DBG("settings init");

    lv_obj_t * list = gui_list_create(par, "System", system_cb);

    gui_list_text_add_entry(list, "Time & date");
    gui_list_text_add_entry(list, "Display");
    gui_list_text_add_entry(list, "Calibration");

    char rev_str[10];
    rew_get_sw_string(rev_str);
    gui_list_info_add_entry(list, "Device info", rev_str);

    return list;
}
