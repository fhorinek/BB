
#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    Glide,
    "Glide ratio",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,
	_b(wf_label_hide),

    lv_obj_t * value;
);


static void Glide_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);

    if (!widget_flag_is_set(slot, wf_label_hide))
    {
		char tmp[16];
		snprintf(tmp, sizeof(tmp), "Glide (%us)", config_get_int(&profile.flight.gr_duration));
		widget_add_title(base, slot, tmp);
    }

    local->value = widget_add_value(base, slot, NULL, NULL);
}

static void Glide_update(widget_slot_t * slot)
{
    char value[8];

    if (isnan(fc.fused.glide_ratio))
        strcpy(value, "---");
    else
    	snprintf(value, sizeof(value), "%0.1f", fc.fused.glide_ratio);

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value, slot->obj);
}


