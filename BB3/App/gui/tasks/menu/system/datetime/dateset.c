
#define DEBUG_LEVEL DBG_DEBUG

#include "dateset.h"

#include "datetime.h"

#include "gui/gui_list.h"

#include "drivers/rtc.h"
#include "etc/epoch.h"

REGISTER_TASK_I(dateset,
    lv_obj_t * day;
    lv_obj_t * month;
    lv_obj_t * year;

    lv_style_t style;
);

static bool dateset_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
    if (event == LV_EVENT_CANCEL)
    {
        lv_obj_t * o = lv_group_get_focused(gui.input.group);

        if (o == local->day)
        {
            gui_switch_task(&gui_datetime, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
        }

        if (o == local->month)
        {
            lv_group_focus_obj(local->day);
            lv_group_set_editing(gui.input.group, true);
        }

        if (o == local->year)
        {
            lv_group_focus_obj(local->month);
            lv_group_set_editing(gui.input.group, true);
        }
    }

    if (event == LV_EVENT_VALUE_CHANGED)
    {
        if (obj == local->month || obj == local->year)
        {

            uint8_t m = lv_roller_get_selected(local->month) + 1;
            uint16_t y = lv_roller_get_selected(local->year) + 2020;

            uint8_t days = datetime_number_of_days(m, y);

            #define OPTION_SIZE 3
            char options[OPTION_SIZE * days + 1];
            for (uint8_t i = 0; i < days; i++)
            {
                sprintf(options + OPTION_SIZE * i, "%02u\n", i + 1);
            }
            options[strlen(options) - 1] = 0;

            uint8_t d = lv_roller_get_selected(local->day);
            if (d >= days)
                d = days - 1;

            lv_roller_set_options(local->day, options, LV_ROLLER_MODE_INIFINITE);
            lv_roller_set_selected(local->day, d, LV_ANIM_OFF);
        }

    }

    if (event == LV_EVENT_CLICKED)
    {
        if (obj == local->day)
        {
            lv_group_focus_obj(local->month);
            lv_group_set_editing(gui.input.group, true);
        }

        if (obj == local->month)
        {
            lv_group_focus_obj(local->year);
            lv_group_set_editing(gui.input.group, true);
        }

        if (obj == local->year)
        {
            uint8_t d = lv_roller_get_selected(local->day) + 1;
            uint8_t m = lv_roller_get_selected(local->month) + 1;
            uint16_t y = lv_roller_get_selected(local->year) + 2020;

            uint8_t wday = datetime_wday_from_epoch(datetime_to_epoch(0, 0, 0, d, m, y));

            rtc_set_date(d, wday, m, y);

            gui_switch_task(&gui_datetime, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
        }

    }

    return false;
}

#define ROLLER_HEIGHT 270

lv_obj_t * dateset_init(lv_obj_t * par)
{
    lv_style_init(&local->style);
    lv_style_set_text_font(&local->style, LV_STATE_DEFAULT, &lv_font_montserrat_28);

    lv_obj_t * list = gui_list_create(par, "Set date", NULL, dateset_cb);
    lv_win_set_layout(list, LV_LAYOUT_PRETTY_MID);

    #define OPTION_SIZE 3
    char options[OPTION_SIZE * 31 + 1];
    for (uint8_t i = 0; i < 31; i++)
    {
        sprintf(options + OPTION_SIZE * i, "%02u\n", i + 1);
    }
    options[strlen(options) - 1] = 0;

    local->day = lv_roller_create(list, NULL);
    lv_obj_add_style(local->day, LV_ROLLER_PART_BG, &local->style);
    lv_obj_set_height(local->day, ROLLER_HEIGHT);

    lv_roller_set_options(local->day, options, LV_ROLLER_MODE_INIFINITE);

    lv_group_add_obj(gui.input.group, local->day);
    lv_obj_set_event_cb(local->day, gui_list_event_cb);


    lv_obj_t * label = lv_label_create(list, NULL);
    lv_obj_add_style(label, LV_LABEL_PART_MAIN, &local->style);
    lv_label_set_text(label, "/");
    lv_obj_set_height(label, ROLLER_HEIGHT);


    for (uint8_t i = 0; i < 12; i++)
    {
        sprintf(options + OPTION_SIZE * i, "%02u\n", i + 1);
    }
    options[strlen(options) - 1] = 0;

    local->month = lv_roller_create(list, NULL);
    lv_obj_add_style(local->month, LV_ROLLER_PART_BG, &local->style);
    lv_obj_set_height(local->month, ROLLER_HEIGHT);

    lv_roller_set_options(local->month, options, LV_ROLLER_MODE_INIFINITE);

    lv_group_add_obj(gui.input.group, local->month);
    lv_obj_set_event_cb(local->month, gui_list_event_cb);


    label = lv_label_create(list, NULL);
    lv_obj_add_style(label, LV_LABEL_PART_MAIN, &local->style);
    lv_label_set_text(label, "/");
    lv_obj_set_height(label, ROLLER_HEIGHT);

    #define YEAR_OPTION_SIZE 5
    for (uint8_t i = 0; i < sizeof(options) / YEAR_OPTION_SIZE; i++)
    {
        sprintf(options + YEAR_OPTION_SIZE * i, "%04u\n", i + 2020);
    }
    options[strlen(options) - 1] = 0;

    local->year = lv_roller_create(list, NULL);
    lv_obj_add_style(local->year, LV_ROLLER_PART_BG, &local->style);
    lv_obj_set_height(local->year, ROLLER_HEIGHT);

    lv_roller_set_options(local->year, options, LV_ROLLER_MODE_INIFINITE);

    lv_group_add_obj(gui.input.group, local->year);
    lv_obj_set_event_cb(local->year, gui_list_event_cb);

    uint8_t d, m;
    uint16_t y;
    rtc_get_date(&d, NULL, &m, &y);

    lv_roller_set_selected(local->day, d - 1, LV_ANIM_OFF);
    lv_roller_set_selected(local->month, m - 1, LV_ANIM_OFF);
    lv_roller_set_selected(local->year, y - 2020, LV_ANIM_OFF);

    lv_group_focus_obj(local->day);
    lv_group_set_editing(gui.input.group, true);

	return list;
}
