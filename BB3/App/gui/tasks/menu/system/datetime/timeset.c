
#define DEBUG_LEVEL DEBUG_DBG

#include "timeset.h"

#include "datetime.h"

#include "gui/gui_list.h"

#include "drivers/rtc.h"
#include "etc/format.h"
#include "etc/timezone.h"

REGISTER_TASK_I(timeset,
    lv_obj_t * hour;
    lv_obj_t * min;

    lv_style_t style;
);

static bool timeset_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
    if (event == LV_EVENT_CANCEL)
    {
        lv_obj_t * o = lv_group_get_focused(gui.input.group);

        if (o == local->hour)
        {
            gui_switch_task(&gui_datetime, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
        }

        if (o == local->min)
        {
            lv_group_focus_obj(local->hour);
            lv_group_set_editing(gui.input.group, true);
        }
    }

    if (event == LV_EVENT_CLICKED)
    {
        if (obj == local->hour)
        {
            lv_group_focus_obj(local->min);
            lv_group_set_editing(gui.input.group, true);
        }

        if (obj == local->min)
        {
            uint8_t h = lv_roller_get_selected(local->hour);
            uint8_t m = lv_roller_get_selected(local->min);

            rtc_set_time(h, m, 0);

            gui_switch_task(&gui_datetime, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
        }

    }

    return false;
}

#define ROLLER_HEIGHT 270

lv_obj_t * timeset_init(lv_obj_t * par)
{
    lv_style_init(&local->style);
    lv_style_set_text_font(&local->style, LV_STATE_DEFAULT, &lv_font_montserrat_34);

    lv_obj_t * list = gui_list_create(par, "Set time", NULL, timeset_cb);
    lv_win_set_layout(list, LV_LAYOUT_PRETTY_MID);

    #define OPTION_SIZE 3
    char options[OPTION_SIZE * 60 + 1];
    for (uint8_t i = 0; i < 24; i++)
    {
        sprintf(options + OPTION_SIZE * i, "%02u\n", i);
    }
    options[OPTION_SIZE * 24 - 1] = 0;

    local->hour = lv_roller_create(list, NULL);
    lv_obj_add_style(local->hour, LV_ROLLER_PART_BG, &local->style);
    lv_obj_set_style_local_margin_left(local->hour, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 20);
    lv_obj_set_height(local->hour, ROLLER_HEIGHT);

    lv_roller_set_options(local->hour, options, LV_ROLLER_MODE_INIFINITE);

    lv_group_add_obj(gui.input.group, local->hour);
    lv_obj_set_event_cb(local->hour, gui_list_event_cb);


    lv_obj_t * label = lv_label_create(list, NULL);
    lv_obj_add_style(label, LV_LABEL_PART_MAIN, &local->style);
    lv_label_set_text(label, ":");
    lv_obj_set_height(label, ROLLER_HEIGHT);


    for (uint8_t i = 23; i < 60; i++)
    {
        sprintf(options + OPTION_SIZE * i, "%02u\n", i);
    }
    options[OPTION_SIZE * 60 - 1] = 0;

    local->min = lv_roller_create(list, NULL);
    lv_obj_add_style(local->min, LV_ROLLER_PART_BG, &local->style);
    lv_obj_set_style_local_margin_right(local->min, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 20);
    lv_obj_set_height(local->min, ROLLER_HEIGHT);

    lv_roller_set_options(local->min, options, LV_ROLLER_MODE_INIFINITE);

    lv_group_add_obj(gui.input.group, local->min);
    lv_obj_set_event_cb(local->min, gui_list_event_cb);


    uint8_t h, m, s;
    rtc_get_time(&h, &m, &s);

    lv_roller_set_selected(local->hour, h, LV_ANIM_OFF);
    lv_roller_set_selected(local->min, m, LV_ANIM_OFF);

    lv_group_focus_obj(local->hour);
    lv_group_set_editing(gui.input.group, true);

	return list;
}
