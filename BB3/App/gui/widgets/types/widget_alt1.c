/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_ISUE
(
    Alt1,
    "Altitude QNH1",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * value;
	lv_obj_t * edit;
	uint32_t last_action;
	uint8_t action_cnt;
);


static void Alt1_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    widget_add_title(base, slot, "Altitude 1");

    char units[8];
    format_altitude_units(units);
    local->value = widget_add_value(base, slot, units, NULL);

    local->edit = NULL;

    local->last_action = 0;
    local->action_cnt = 0;
}

static void Alt1_edit(widget_slot_t * slot, uint8_t action)
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
			local->edit = widget_create_edit_overlay("Altitude 1", "Set QNH1");
			lv_obj_t * base = widget_edit_overlay_get_base(local->edit);
			lv_obj_t * qnh = lv_label_create(base, NULL);
			lv_obj_set_style_local_text_font(qnh, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, gui.styles.widget_fonts[1]);
			lv_obj_t * alt = lv_label_create(base, NULL);
			lv_obj_set_style_local_text_font(alt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, gui.styles.widget_fonts[1]);

			//update values
			Alt1_update(slot);
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

		INFO("AC %u %lu", local->action_cnt, HAL_GetTick() - local->last_action);

		local->last_action = HAL_GetTick();

		int8_t diff = 0;
		if (action == WIDGET_ACTION_LEFT)
			diff = -1;

		if (action == WIDGET_ACTION_RIGHT)
			diff = 1;

		if (diff != 0)
		{
			diff *= 1 + (local->action_cnt / 5);

			float alt = fc.fused.altitude1 + diff;
			uint32_t new = fc_alt_to_qnh(alt, fc.fused.pressure);

			config_set_big_int(&config.vario.qnh1, new);
			new = config_get_big_int(&config.vario.qnh1);

			fc_manual_alt1_change(alt);
		}

    }
}

static void Alt1_update(widget_slot_t * slot)
{
    if (fc.fused.status == fc_dev_ready)
    {
        char value[10];

        format_altitude(value, fc.fused.altitude1);
        lv_label_set_text(local->value, value);
    }
    else
    {
        lv_label_set_text(local->value, "---");
    }
    widget_update_font_size(local->value, slot->obj);

    if (local->edit != NULL)
    {
		lv_obj_t * base = widget_edit_overlay_get_base(local->edit);
		lv_obj_t * alt = lv_obj_get_child(base, NULL);
		lv_obj_t * qnh = lv_obj_get_child(base, alt);

		char buff[32];
		char val[16];
		char units[8];

		format_altitude(val, fc.fused.altitude1);
		format_altitude_units(units);
		snprintf(buff, sizeof(buff), "%s %s", val, units);
		lv_label_set_text(alt, buff);

		snprintf(buff, sizeof(buff), "%lu Pa", config_get_big_int(&config.vario.qnh1));
		lv_label_set_text(qnh, buff);    }
}

static void Alt1_stop(widget_slot_t * slot)
{
    if (local->edit != NULL)
    {
        widget_destroy_edit_overlay(local->edit);
    }
}

