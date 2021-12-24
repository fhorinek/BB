/*
 * widget_thermal_path.c
 *
 *  Created on: 12. 12. 2021
 *      Author: thrull
 */

#include "gui/widgets/widget.h"
#include "fc/fc.h"
#include "etc/geo_calc.h"

#define THERMAL_PATH_POSITIONS 30
#define THERMAL_DOT_SIZE 6

REGISTER_WIDGET_IU
(
    TPath,
    "Thermal path",
    120,
    120,
	_b(wf_label_hide),

    lv_obj_t * text;
    lv_obj_t * thermals[THERMAL_PATH_POSITIONS];

    lv_obj_t * arrow;
    lv_obj_t * path;
    lv_obj_t * dot;

    lv_point_t arrow_points[WIDGET_ARROW_POINTS];
    lv_point_t thermal_path_points[THERMAL_PATH_POSITIONS];

);


static void TPath_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "Thermal path");

    local->text = widget_add_value(base, slot, NULL, NULL);
    local->arrow = widget_add_arrow(base, slot, local->arrow_points, NULL, NULL);

	local->dot = lv_obj_create(slot->obj, NULL);
	lv_obj_set_size(local->dot, 10, 10);
	lv_obj_align(local->dot, slot->obj, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_color(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_PURPLE);
	lv_obj_set_style_local_radius(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);

	 for (uint8_t i = 0; i < THERMAL_PATH_POSITIONS; i++)
	 {
		 local->thermals[i] = lv_obj_create(slot->obj, NULL);
		 lv_obj_set_style_local_radius(local->thermals[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, THERMAL_DOT_SIZE);
		 lv_obj_set_style_local_bg_color(local->thermals[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		 lv_obj_set_size(local->thermals[i], THERMAL_DOT_SIZE, THERMAL_DOT_SIZE);
		 lv_obj_set_auto_realign(local->thermals[i], true);
	 }

	 lv_obj_set_hidden(local->thermals[0], true);
}



static void TPath_update(widget_slot_t * slot)
{
	if (fc.flight.mode == flight_flight && fc.gnss.fix > 0) {
		//draw last THERMAL_PATH_POSITIONS positions

    	int32_t curr_lat = fc.gnss.latitude;
    	int32_t curr_lon = fc.gnss.longtitude;

		if (fc.history.size > THERMAL_PATH_POSITIONS) {

			//uint32_t size = THERMAL_PATH_POSITIONS;
			uint32_t size = fc.history.size;//min(fc.history.size, (THERMAL_PATH_POSITIONS * 1000) / FC_HISTORY_PERIOD);
			//uint16_t step = 1000 / FC_HISTORY_PERIOD; // 1/4 s in history
			int32_t x;
			int32_t y;
			int32_t last_hpos_lon = 0;
			int32_t last_hpos_lat = 0;
			uint16_t t = 0;
			uint16_t i = 0;

			while (i < size) {
			//for (uint16_t i = 0; i < size; i++) {
				uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i) % FC_HISTORY_SIZE;
				fc_pos_history_t h_pos = fc.history.positions[index];

				i++;

				if (i % 4 != 0)
					continue;

				if (i >= THERMAL_PATH_POSITIONS)
					break;

				geo_to_pix_w_h(curr_lon, curr_lat, 0, h_pos.lon, h_pos.lat, &x, &y, slot->w, slot->h);

				lv_obj_align(local->thermals[t], local->text, LV_ALIGN_CENTER, x-(slot->w/2), y-(slot->h/2));

				if (h_pos.vario < -1.5) {
					lv_obj_set_style_local_bg_color(local->thermals[t], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);
				} else if (h_pos.vario < 0.0) {
					lv_obj_set_style_local_bg_color(local->thermals[t], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
				} else if (h_pos.vario < 0.5) {
					lv_obj_set_style_local_bg_color(local->thermals[t], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
				} else if (h_pos.vario > 0.5) {
					lv_obj_set_style_local_bg_color(local->thermals[t], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
				} else if (h_pos.vario > 1.2) {
					lv_obj_set_style_local_bg_color(local->thermals[t], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

				}

				//lv_obj_invalidate(local->thermals[t]);

				last_hpos_lon = h_pos.lon;
				last_hpos_lat = h_pos.lat;

				t++;
			}
		}
	}

	if (fc.gnss.fix == 0)
	{
		lv_obj_set_hidden(local->arrow, true);
		lv_obj_set_hidden(local->dot, false);
	}
	else
	{
		if (fc.gnss.ground_speed > 2)
		{
			widget_arrow_rotate_size(local->arrow, local->arrow_points, fc.gnss.heading, 40);
			lv_obj_set_hidden(local->arrow, false);
			lv_obj_set_hidden(local->dot, true);
		}
		else
		{
			lv_obj_set_hidden(local->arrow, true);
			lv_obj_set_hidden(local->dot, false);
		}
	}
}
