/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"
#include "widget_alt.h"

typedef struct
{
	lv_obj_t * value;
	lv_obj_t * edit;
	uint32_t last_action;
	uint8_t action_cnt;
} widget_local_vars_t;

#undef local
#define local ((widget_local_vars_t *)local_ptr)

void Alt_init(void * local_ptr, lv_obj_t * base, widget_slot_t * slot, char * title)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, title);

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

    local->edit = NULL;

    local->last_action = 0;
    local->action_cnt = 0;
}


void Alt_edit(void * local_ptr, widget_slot_t * slot, uint8_t action, char * title, float * alt_ptr, cfg_entry_t * qnh_cfg)
{
    if (action == WIDGET_ACTION_CLOSE)
    {
        if (local->edit != NULL)
        {
            widget_destroy_edit_overlay(local->edit);
            local->edit = NULL;
        }

        return;
    }

    if (action == WIDGET_ACTION_MIDDLE)
    {
    	widget_destroy_edit_overlay(local->edit);
    	local->edit = NULL;
    }

    if (action == WIDGET_ACTION_LEFT || action == WIDGET_ACTION_RIGHT || action == WIDGET_ACTION_HOLD)
    {
    	if (fc.fused.status != fc_dev_ready)
    	{
    		return;
    	}

		if (local->edit == NULL)
		{
			//create menu
			local->edit = widget_create_edit_overlay(title, _("Set QNH"));
			lv_obj_t * base = widget_edit_overlay_get_base(local->edit);
			lv_obj_t * qnh = lv_label_create(base, NULL);
			lv_obj_set_style_local_text_font(qnh, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, gui.styles.widget_fonts[FONT_XL]);
			lv_obj_t * alt = lv_label_create(base, NULL);
			lv_obj_set_style_local_text_font(alt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, gui.styles.widget_fonts[FONT_XL]);

			//update values
			Alt_update(local_ptr, slot, *alt_ptr, config_get_big_int(qnh_cfg));
		}

		if (HAL_GetTick() - local->last_action < 300)
		{
			if (local->action_cnt < 50)
				local->action_cnt++;
		}
		else
		{
			local->action_cnt = 0;;
		}

		//INFO("AC %u %lu", local->action_cnt, HAL_GetTick() - local->last_action);

		local->last_action = HAL_GetTick();

		int8_t diff = 0;
		if (action == WIDGET_ACTION_LEFT)
			diff = -1;

		if (action == WIDGET_ACTION_RIGHT)
			diff = 1;

		if (diff != 0)
		{
			diff *= 1 + (local->action_cnt / 5);

			float new_alt = *alt_ptr + diff;
			uint32_t new = fc_alt_to_qnh(new_alt, fc.fused.pressure);

			config_set_big_int(qnh_cfg, new);

			if (alt_ptr == &fc.fused.altitude1)
				fc_manual_alt1_change(new_alt);

			widget_reset_edit_overlay_timer();
		}

    }
}

void Alt_update(void * local_ptr, widget_slot_t * slot, float alt_val, int32_t qnh_val)
{
    uint8_t units = config_get_select(&config.units.altitude);
    if (widget_flag_is_set(slot, wf_alt_unit))
    {
        if (units == ALTITUDE_M)
            units = ALTITUDE_FT;
        else if (units == ALTITUDE_FT)
            units = ALTITUDE_M;
    }

    if (fc.fused.status == fc_dev_ready)
    {
        char value[10];

        format_altitude_2(value, alt_val, units);
        lv_label_set_text(local->value, value);
    }
    else
    {
        lv_label_set_text(local->value, "---");
    }
    widget_update_font_size(local->value);

    if (local->edit != NULL)
    {
		lv_obj_t * base = widget_edit_overlay_get_base(local->edit);
		lv_obj_t * alt = lv_obj_get_child(base, NULL);
		lv_obj_t * qnh = lv_obj_get_child(base, alt);

		char buff[32];

		format_altitude_with_units_2(buff, alt_val, units);
		lv_label_set_text(alt, buff);

		snprintf(buff, sizeof(buff), "%lu Pa", qnh_val);
		lv_label_set_text(qnh, buff);
    }
}

 void Alt_stop(void * local_ptr, widget_slot_t * slot)
{
    if (local->edit != NULL)
    {
        widget_destroy_edit_overlay(local->edit);
    }
}

