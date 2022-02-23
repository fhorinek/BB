/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"
#include "fc/agl.h"

REGISTER_WIDGET_IU
(
    TGain,
    "Thermal - gain",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_units_hide) | _b(wf_alt_unit),

    lv_obj_t * value;
);


static void TGain_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "Thermal gain");

    char tmp[8];
    char * uni = tmp;
    if (widget_flag_is_set(slot, wf_units_hide))
    {
        uni = NULL;
    }
    else
    {
        uint8_t units = config_get_select(&config.units.altitude);
        if (widget_flag_is_set(slot, wf_alt_unit))
        {
            if (units == ALTITUDE_M)
                units = ALTITUDE_FT;
            else if (units == ALTITUDE_FT)
                units = ALTITUDE_M;
        }

        format_altitude_units_2(tmp, units);
    }

    local->value = widget_add_value(base, slot, uni, NULL);
}

static void TGain_update(widget_slot_t * slot)
{
    uint8_t units = config_get_select(&config.units.altitude);
    if (widget_flag_is_set(slot, wf_alt_unit))
    {
        if (units == ALTITUDE_M)
            units = ALTITUDE_FT;
        else if (units == ALTITUDE_FT)
            units = ALTITUDE_M;
    }

    char value[16];

    format_altitude_gain_2(value, fc.flight.circling_gain, units);
    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value);
}


