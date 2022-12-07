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
    Agl,
    "Height - above ground",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_units_hide),

    lv_obj_t * value;
);


static void Agl_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, NULL);

    char tmp[8];
    char * units = tmp;
    if (widget_flag_is_set(slot, wf_units_hide))
    	units = NULL;
    else
    	format_altitude_units(tmp);

	local->value = widget_add_value(base, slot, units, NULL);
}

static void Agl_update(widget_slot_t * slot)
{
    char value[30];

    if (fc.gnss.fix == 0)
    	strcpy(value, _("No\nGNSS"));
    else if (fc.gnss.fix != 3)
        strcpy(value, _("Need\n3D fix"));
    else if (fc.agl.agl == AGL_INVALID)
        strcpy(value, _("No TOPO\ndata"));
    else
        format_altitude(value, fc.agl.agl);

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value);
}


