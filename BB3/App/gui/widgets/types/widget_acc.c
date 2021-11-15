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
    Acc,
    "G-meter",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_units_hide),

    lv_obj_t * value;
);


static void Acc_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "G-meter");

    if (widget_flag_is_set(slot, wf_units_hide))
    	local->value = widget_add_value(base, slot, NULL, NULL);
    else
    	local->value = widget_add_value(base, slot, "g", NULL);
}

static void Acc_update(widget_slot_t * slot)
{
    char value[16];

	if (fc.imu.status == fc_device_not_calibrated)
	{
		strcpy(value, "Need\nCalib.");
	}
	else
	{
		bool found = false;
		int16_t accel = 0;
		uint16_t size = min(fc.history.size, (config_get_int(&profile.flight.acc_duration) * 1000) / FC_HISTORY_PERIOD);

		for (uint16_t i = 0; i < size; i++)
		{
			uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i) % FC_HISTORY_SIZE;
			if (fc.history.positions[index].flags & FC_POS_HAVE_ACC)
			{
				accel = max(accel, fc.history.positions[index].accel);
				found = true;
			}
		}

		if (found)
		{
			snprintf(value, sizeof(value), "%0.1f", accel / 1000.0);
		}
		else
		{
			strcpy(value, "---");
		}
	}

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value);
}


