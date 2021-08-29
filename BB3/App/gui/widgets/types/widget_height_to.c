
#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    HeightTO,
    "Height above Take-off",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * value;
);


static void HeightTO_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    widget_add_title(base, slot, "Take-off");

    char units[8];
    format_altitude_units(units);
    local->value = widget_add_value(base, slot, units, NULL);
}

static void HeightTO_update(widget_slot_t * slot)
{
    char value[8];

    if (fc.flight.mode == flight_flight || fc.flight.mode == flight_landed)
    	format_altitude(value, fc.fused.altitude1 - fc.flight.start_alt);
    else
        strcpy(value, "---");

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value, slot->obj);
}


