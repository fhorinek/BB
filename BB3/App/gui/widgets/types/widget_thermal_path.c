/*
 * widget_thermal_path.c
 *
 *  Created on: 12. 12. 2021
 *      Author: thrull
 */

#include "gui/widgets/widget.h"
#include "fc/fc.h"
#include "etc/geo_calc.h"

#define THERMAL_DOTS_POSITIONS 30
#define THERMAL_DOT_SIZE 6
#define THERMAL_HISTORY_STEP 4  // GPS 1/s history 4/s

//static

REGISTER_WIDGET_IU
(
    TDots,
    "Thermal dots",
    120,
    120,
	_b(wf_label_hide),

    lv_obj_t * text;
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

    local->arrow = widget_add_arrow(base, slot, local->arrow_points, NULL, NULL);

    local->text = lv_label_create(slot->obj, NULL);
    lv_obj_align(local->text, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
	lv_obj_set_style_local_bg_color(local->text, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

	local->dot = lv_obj_create(slot->obj, NULL);
	lv_obj_set_size(local->dot, 10, 10);
	lv_obj_align(local->dot, slot->obj, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_color(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_PURPLE);
	lv_obj_set_style_local_radius(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);

	 for (uint8_t i = 0; i < THERMAL_DOTS_POSITIONS; i++)
	 {
		 local->thermals[i] = lv_obj_create(slot->obj, NULL);
		 lv_obj_set_style_local_radius(local->thermals[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, THERMAL_DOT_SIZE);
		 lv_obj_set_size(local->thermals[i], THERMAL_DOT_SIZE, THERMAL_DOT_SIZE);
		 lv_obj_set_auto_realign(local->thermals[i], true);
		 lv_obj_align(local->thermals[i], slot->obj, LV_ALIGN_IN_TOP_LEFT, -1, -1);
		 lv_obj_set_style_local_bg_color(local->thermals[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	 }
}

static void TDots_update(widget_slot_t * slot)
{
	uint32_t start = HAL_GetTick();

	// render dots only if in flight & with enough data to display
	if (fc.flight.mode == flight_flight && fc.gnss.fix > 0 && ((fc.history.size / THERMAL_HISTORY_STEP) >= THERMAL_DOTS_POSITIONS)) {
    	int32_t curr_lat = fc.gnss.latitude;
    	int32_t curr_lon = fc.gnss.longtitude;

    	int16_t x, y;

    	int16_t i = THERMAL_HISTORY_STEP * 2; // skip last history position?
		int16_t t = 0;

		while (i < fc.history.size) {
			uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i) % FC_HISTORY_SIZE;
			fc_pos_history_t h_pos = fc.history.positions[index];

			i += THERMAL_HISTORY_STEP;

			if (t >= THERMAL_DOTS_POSITIONS)
				break;

			geo_to_pix_w_h(curr_lon, curr_lat, 0, h_pos.lon, h_pos.lat, &x, &y, slot->w, slot->h);

			lv_color_t c = LV_COLOR_WHITE;

			if (h_pos.vario < -220) {
				c = LV_COLOR_BLUE;
			} else if (h_pos.vario < -120) {
				c = LV_COLOR_NAVY;
			} else if (h_pos.vario < 0) {
				c = LV_COLOR_WHITE;
			} else if (h_pos.vario < 100) {
				c = LV_COLOR_YELLOW;
			} else if (h_pos.vario < 160) {
				c = LV_COLOR_ORANGE;
			} else if (h_pos.vario >= 160) {
				c = LV_COLOR_RED;
			}

			lv_obj_set_style_local_bg_color(local->thermals[t], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, c);

			lv_obj_align(local->thermals[t], slot->obj, LV_ALIGN_CENTER, x-(slot->w/2), y-(slot->h/2));

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
	// DEBUG
	uint32_t end = HAL_GetTick();
	char value[16];
	sprintf(value, "%u ms",  (unsigned int) end-start);
	lv_label_set_text(local->text, value);
}
