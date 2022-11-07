
#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    HeightTO,
    "Height - above TO",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_units_hide),

    lv_obj_t * value;
);


static void HeightTO_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, _("Take-off"));

    char tmp[8];
    char * units = tmp;
    if (widget_flag_is_set(slot, wf_units_hide))
    	units = NULL;
    else
    	format_altitude_units(tmp);

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
    widget_update_font_size(local->value);
}


