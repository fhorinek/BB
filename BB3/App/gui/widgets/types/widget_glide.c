
#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    Glide,
    _i("Glide ratio"),
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_glide_show_avg_vario) | _b(wf_units_hide),

    lv_obj_t * value;
    lv_obj_t * units;
    bool in_vario_mode;
);

static void switch_to_vario(widget_slot_t * slot)
{
    if (local->in_vario_mode == true)
        return;

    local->in_vario_mode = true;

    if (slot->title != NULL)
    {
        lv_label_set_text_fmt(slot->title, _("Avg (%us)"), config_get_int(&profile.vario.avg_duration));
    }

    if (local->units != NULL)
    {
        lv_obj_set_hidden(local->units, false);
        int16_t h = lv_obj_get_height(local->value) - lv_obj_get_height(local->units);
        lv_obj_set_height(local->value, h);
    }
}

static void switch_to_glide(widget_slot_t * slot)
{
    if (local->in_vario_mode == false)
        return;

    local->in_vario_mode = false;

    if (slot->title != NULL)
    {
        lv_label_set_text_fmt(slot->title, _("Glide (%us)"), config_get_int(&profile.flight.gr_duration));
    }


    if (local->units != NULL)
    {
        lv_obj_set_hidden(local->units, true);
        int16_t h = lv_obj_get_height(local->value) + lv_obj_get_height(local->units);
        lv_obj_set_height(local->value, h);
    }
}

static void Glide_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);

    if (!widget_flag_is_set(slot, wf_label_hide))
    {
		widget_add_title(base, slot, "");
		lv_obj_set_auto_realign(slot->title, true);
    }

    char tmp[8];
    char * units = tmp;
    if (widget_flag_is_set(slot, wf_units_hide))
        units = NULL;
    else
        format_vario_units(tmp);

    local->value = widget_add_value(base, slot, units, &local->units);

    local->in_vario_mode = true;
    switch_to_glide(slot);
}

static void Glide_update(widget_slot_t * slot)
{
    char value[8];

    if (isnan(fc.fused.glide_ratio))
    {
        if (widget_flag_is_set(slot, wf_glide_show_avg_vario))
        {
            switch_to_vario(slot);
            if (fc.fused.status != fc_dev_ready)
                strcpy(value, "---");
            else
                format_vario(value, fc.fused.avg_vario);
        }
        else
        {
            switch_to_glide(slot);
            strcpy(value, "---");
        }
    }
    else
    {
        switch_to_glide(slot);
    	snprintf(value, sizeof(value), "%0.1f", fc.fused.glide_ratio);
    }

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value);
}


