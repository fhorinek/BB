/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"
#include "fc/fc.h"

REGISTER_WIDGET_IUE
(
    FTime,
    "Flight time",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * value;
    lv_obj_t * sub;
);

static void FTime_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    widget_add_title(base, slot, "Flight time");

    local->value = widget_add_value(base, slot, "", &local->sub);
}

static void FTime_edit(widget_slot_t * slot, uint8_t action)
{
    if (action == WIDGET_ACTION_HOLD)
    {
        if (fc.flight.mode == flight_wait_to_takeoff)
            fc_takeoff();
        else if (fc.flight.mode == flight_flight)
            fc_landing();
        else if (fc.flight.mode == flight_landed)
            fc_reset();
    }
}

static void FTime_update(widget_slot_t * slot)
{
    if (fc.flight.mode == flight_not_ready)
    {
        lv_label_set_text(local->value, "---");
        lv_label_set_text(local->sub, "");
    }
    else if (fc.flight.mode == flight_wait_to_takeoff)
    {
        lv_label_set_text(local->value, "Start");
        lv_label_set_text(local->sub, "ready");
    }
    else if (fc.flight.mode == flight_flight)
    {
        char value[8];
        uint32_t delta = (HAL_GetTick() - fc.flight.start_time) / 1000;
        uint8_t hour = delta / 3600;
        delta %= 3600;
        uint8_t min = delta / 60;
        delta %= 60;
        uint8_t sec = delta;

        if (hour == 0)
            snprintf(value, sizeof(value), "%02u.%02u", min, sec);
        else
            snprintf(value, sizeof(value), "%02u:%02u", hour, min);

        lv_label_set_text(local->value, value);
        lv_label_set_text(local->sub, "in flight");
    }
    else if (fc.flight.mode == flight_landed)
    {
        char value[8];
        uint32_t delta = fc.flight.duration;
        uint8_t hour = delta / 3600;
        delta %= 3600;
        uint8_t min = delta / 60;
        delta %= 60;
        uint8_t sec = delta;

        if (hour == 0)
            snprintf(value, sizeof(value), "%02u.%02u", min, sec);
        else
            snprintf(value, sizeof(value), "%02u:%02u", hour, min);

        lv_label_set_text(local->value, value);
        lv_label_set_text(local->sub, "landed");
    }

    widget_update_font_size(local->value, slot->obj);
}


