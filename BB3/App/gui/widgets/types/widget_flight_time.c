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
    _i("Time - flight"),
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_units_hide),

    lv_obj_t * value;
    lv_obj_t * sub;
);

static void FTime_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, _("Flight time"));

    char * sub_text = "";
    if (widget_flag_is_set(slot, wf_units_hide))
        sub_text = NULL;

    local->value = widget_add_value(base, slot, sub_text, &local->sub);
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
        	fc_takeoff();
    }
}

static void FTime_update(widget_slot_t * slot)
{
    if (fc.flight.mode == flight_not_ready)
    {
        lv_label_set_text(local->value, "---");
        if (local->sub != NULL)
            lv_label_set_text(local->sub, _("wait"));
    }
    else if (fc.flight.mode == flight_wait_to_takeoff)
    {
        lv_label_set_text(local->value, _("Start"));
        if (local->sub != NULL)
            lv_label_set_text(local->sub, _("ready"));
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
        if (local->sub != NULL)
            lv_label_set_text(local->sub, _("in flight"));
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
        if (local->sub != NULL)
            lv_label_set_text(local->sub, _("landed"));
    }

    widget_update_font_size(local->value);
}


