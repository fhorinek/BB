
#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    Glide,
    "Glide ratio",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * value;
);


static void Glide_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    widget_add_title(base, slot, NULL);

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


