/*
 * widget_map.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG
#include "gui/widgets/widget.h"
#include "gui/map/map_thread.h"
#include "gui/map/tile.h"
#include "gui/map/map_obj.h"

#include "etc/geo_calc.h"

REGISTER_WIDGET_ISUE(Map,
    "Map",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	0,

	map_obj_data_t data;
	lv_obj_t *edit;
	uint8_t action_cnt;
	uint32_t last_action;
);

static void Map_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);

    local->edit = NULL;
    local->last_action = 0;
    local->action_cnt = 0;

    map_obj_init(slot->obj, &local->data);
}


static void Map_update(widget_slot_t * slot)
{
    int32_t disp_lat;
    int32_t disp_lon;
    int16_t zoom = config_get_int(&profile.map.zoom_flight);

    if (fc.gnss.fix == 0)
    {
        disp_lat = config_get_big_int(&profile.ui.last_lat);
        disp_lon = config_get_big_int(&profile.ui.last_lon);
    }
    else
    {
        disp_lat = fc.gnss.latitude;
        disp_lon = fc.gnss.longtitude;
    }

	map_obj_loop(&local->data, disp_lat, disp_lon);
	map_obj_glider_loop(&local->data);
	map_obj_fanet_loop(&local->data, disp_lat, disp_lon, zoom);

    if (local->edit != NULL)
     {
     	lv_obj_t * base = widget_edit_overlay_get_base(local->edit);
     	lv_obj_t * zoom = lv_obj_get_child(base, lv_obj_get_child(base, NULL));
      	char buff[32];
      	uint16_t zoom_p = pow(2, config_get_int(&profile.map.zoom_flight));
      	int32_t guide_m = (zoom_p * 111000 * 120 / MAP_DIV_CONST);
      	format_distance_with_units2(buff, guide_m);
     	lv_label_set_text(zoom, buff);
     }
}

static void Map_stop(widget_slot_t * slot)
{
    if (local->edit != NULL)
    {
        widget_destroy_edit_overlay(local->edit);
    }
}

static void Map_edit(widget_slot_t * slot, uint8_t action)
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
		if (local->edit == NULL)
		{
			//create menu
			local->edit = widget_create_edit_overlay("", "Set Zoom Level");

            //no anim, make fully transparent
            lv_anim_get(local->edit, NULL);
            lv_obj_set_style_local_bg_opa(local->edit, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

			lv_obj_t * base = widget_edit_overlay_get_base(local->edit);
			lv_obj_t * zoom = lv_label_create(base, NULL);
            lv_obj_set_style_local_pad_top(zoom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 15);
            lv_obj_set_style_local_text_font(zoom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, gui.styles.widget_fonts[FONT_L]);

			lv_obj_t * bar = lv_obj_create(base, NULL);
			lv_obj_set_size(bar, 120, 8);
            lv_obj_set_style_local_bg_color(bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_border_color(bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            lv_obj_set_style_local_border_width(bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 1);

            lv_obj_t * bar2 = lv_obj_create(bar, NULL);
            lv_obj_set_size(bar2, 40, 8);
            lv_obj_align(bar2, bar, LV_ALIGN_CENTER, 0, 0);

			//update values
			Map_update(slot);
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
			diff = +1;

		if (action == WIDGET_ACTION_RIGHT)
			diff = -1;

		if (diff != 0)
		{
			diff *= 1 + (local->action_cnt / 5);

			int16_t zoom = config_get_int(&profile.map.zoom_flight);
                        int16_t new = zoom + diff;
			config_set_int(&profile.map.zoom_flight, new);

			widget_reset_edit_overlay_timer();
		}

    }
}
