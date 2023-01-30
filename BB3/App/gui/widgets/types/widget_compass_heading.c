/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
	CompPoints,
	_i("Compass - points"),
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide),

    lv_obj_t * value;
);

static void CompPoints_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, _("Compass"));

    local->value = widget_add_value(base, slot, NULL, NULL);

}

static void CompPoints_update(widget_slot_t * slot)
{
    char value[16];

    if (fc.imu.status == fc_device_not_calibrated)
    {
		lv_label_set_text(local->value, _("Need\nCalib."));
    }
    else
    {
		format_hdg_to_points(value, fc.fused.azimuth_filtered);
		lv_label_set_text(local->value, value);
    }
    widget_update_font_size(local->value);
}


