/*
 * widget_thermal_path.c
 *
 *  Created on: 12. 12. 2021
 *      Author: thrull
 */

#include "gui/widgets/widget.h"
#include "fc/fc.h"
#include "etc/geo_calc.h"

#define THERMAL_DOTS_POSITIONS 60
#define THERMAL_DOT_SIZE 6
#define THERMAL_HISTORY_STEP 4  // GPS 1/s history 4/s
#define THERMAL_ZOOM_THRESHOLD_GS 18.0 // GS above zoom

REGISTER_WIDGET_IU
(
    TDots,
    "Thermal dots",
    120,
    120,
	_b(wf_label_hide),

    lv_obj_t * thermals[THERMAL_DOTS_POSITIONS];

    lv_obj_t * arrow;
    lv_obj_t * dot;

    lv_point_t arrow_points[WIDGET_ARROW_POINTS];
);


static void TDots_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "Thermal dots");

	 for (uint8_t i = 0; i < THERMAL_DOTS_POSITIONS; i++)
	 {
		 local->thermals[i] = lv_obj_create(slot->obj, NULL);
		 lv_obj_set_style_local_radius(local->thermals[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, THERMAL_DOT_SIZE);
		 lv_obj_set_size(local->thermals[i], THERMAL_DOT_SIZE, THERMAL_DOT_SIZE);
		 lv_obj_set_auto_realign(local->thermals[i], true);
		 lv_obj_align(local->thermals[i], slot->obj, LV_ALIGN_IN_TOP_LEFT, -1, -1);
		 lv_obj_set_style_local_bg_color(local->thermals[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	 }

    local->arrow = widget_add_arrow(base, slot, local->arrow_points, NULL, NULL);

	local->dot = lv_obj_create(slot->obj, NULL);
	lv_obj_set_size(local->dot, 10, 10);
	lv_obj_align(local->dot, local->arrow, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_color(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_PURPLE);
	lv_obj_set_style_local_radius(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);

}

static void TDots_update(widget_slot_t * slot)
{
	// render dots only if in flight & with enough data to display
	if (fc.flight.mode == flight_flight && fc.gnss.fix > 0) {
		//&& ((fc.history.size / THERMAL_HISTORY_STEP) >= THERMAL_DOTS_POSITIONS)
    	int32_t curr_lat = fc.gnss.latitude;
    	int32_t curr_lon = fc.gnss.longtitude;

    	int16_t x, y;
    	uint8_t zoom = (fc.flight.circling ? 0 : 1);

    	int16_t i = THERMAL_HISTORY_STEP * 2; // skip last 2 history positions
		int16_t t = 0;

		while (i < fc.history.size) {
			uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i) % FC_HISTORY_SIZE;
			fc_pos_history_t h_pos = fc.history.positions[index];

			i += THERMAL_HISTORY_STEP;

			if (t >= THERMAL_DOTS_POSITIONS)
				break;

			geo_to_pix_w_h(curr_lon, curr_lat, zoom, h_pos.lon, h_pos.lat, &x, &y, slot->w, slot->h);

			lv_color_t c = LV_COLOR_SILVER;

			if (h_pos.vario < -60) {
				c = LV_COLOR_GRAY;
			} else if (h_pos.vario < 0) {
				c = LV_COLOR_SILVER;
			} else if (h_pos.vario < 50) {
				c = LV_COLOR_MAKE(0x11, 0xcc, 0x11);
			} else if (h_pos.vario < 100) {
				c = LV_COLOR_MAKE(0x00, 0xAF, 0x00);
			} else if (h_pos.vario < 150) {
				c = LV_COLOR_GREEN;
			} else if (h_pos.vario < 220) {
				c = LV_COLOR_MAKE(0xFF, 0x6F, 0x00);
			} else if (h_pos.vario >= 250) {
				c = LV_COLOR_MAKE(0xFF, 0x1A, 0x00);
			}

			lv_obj_set_style_local_bg_color(local->thermals[t], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, c);

			lv_obj_align(local->thermals[t], local->arrow, LV_ALIGN_CENTER, x - (slot->w / 2), y - (slot->h / 2));

			t++;
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
