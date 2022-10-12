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
#include "fc/recorder.h"

#include <inttypes.h>

#include "etc/geo_calc.h"

#define POINT_NUM 100

REGISTER_WIDGET_ISUE(Map,
    "Map",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	0,

	map_obj_data_t data;
	lv_obj_t *edit;
	uint8_t action_cnt;
	uint32_t last_action;

	int16_t previous_pnum;
	lv_point_t previous_mastertile_pos;

	lv_point_t p[POINT_NUM];
	lv_obj_t *line;
);

static void Map_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);

    local->edit = NULL;
    local->last_action = 0;
    local->action_cnt = 0;
    local->line = NULL;
    local->previous_pnum = INT16_MAX;

    map_obj_init(slot->obj, &local->data);

	local->line = lv_line_create(local->data.map, NULL);
	lv_obj_set_style_local_line_color(local->line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);
	lv_obj_set_style_local_line_width(local->line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
}

/**
 * Show a trail after the glider on the map.
 *
 * @param disp_lat the latitude of the center of the map
 * @param disp_lon the longitude of the center of the map
 * @param zoom current zoom level
 * @param slot the slot of the widget
 */
void compute_trail(int32_t disp_lat, int32_t disp_lon, int16_t zoom, widget_slot_t * slot)
{
	fc_rec_entry_t *p, *p_start;
	size_t p_num;
	float p_i, step;
	int point_i;
	int16_t x,y;
	int16_t w,h;
	bool line_changed;
	lv_point_t mastertile_pos;

	line_changed = false;

	// Check if flight recorder has changed or map has changed
	p_num = fc_recorder_get_recorded_number();
	if ( p_num != local->previous_pnum ) 
		line_changed = true;
	else
	{
		map_get_master_tile_xy(local, &mastertile_pos);
		if ( local->previous_mastertile_pos.x != mastertile_pos.x ||
			 local->previous_mastertile_pos.y != mastertile_pos.y )
		{
			line_changed = true;
			local->previous_mastertile_pos = mastertile_pos;
		}
	}

	if ( line_changed )
	{
		p_start = fc_recorder_get_start();
		step = (float)p_num / POINT_NUM;
		if ( step < 1 ) step = 1;
		point_i = 0;

		w = lv_obj_get_width(local->data.map);
		h = lv_obj_get_height(local->data.map);

		for ( p_i = 0; p_i < p_num; p_i += step )
		{
			p = p_start + (int)p_i;

			//DBG("lon: %" PRId32 " lat %" PRId32, p->lon, p->lat);
			geo_to_pix_w_h(disp_lon, disp_lat, zoom, p->lon, p->lat, &x, &y, w, h);

			local->p[point_i].x = x;
			local->p[point_i].y = y;
			point_i++;
			if ( point_i >= POINT_NUM) break;
		}
		if ( point_i > 2 )
			lv_line_set_points(local->line, local->p, point_i - 1);
	}

#if 0
	DBG("compute_trail: %d points, recorder %d", point_i, p_num);
	for ( int i = 0; i < point_i; i++ )
	{
		DBG("P%d %d,%d", i, local->p[i].x, local->p[i].y);
	}
#endif
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

	if ( config_get_bool(&profile.map.show_glider_trail) )
		compute_trail(disp_lat, disp_lon, zoom, slot);

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
