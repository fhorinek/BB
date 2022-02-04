/*
 * widget_altitude_graph.c
 *
 *  Created on: 31. 1. 2022
 *      Author: tilmann
 */

// #define DEBUG_LEVEL DBG_DEBUG

#include <limits.h>
#include "gui/widgets/widget.h"
#include "drivers/power/pwr_mng.h"

#include "../../images/glider/glider-20x21.h"

// Number of seconds, that we plot in this graph
#define NUM_PLOTS 30

#define GRAPH_HISTORY_STEP (1000 / FC_HISTORY_PERIOD)

REGISTER_WIDGET_ISU
(
    AltitudeGraph,
    "Altitude Graph",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide),

    lv_obj_t * canvas;        // Canvas to paint to
    lv_color_t *cbuf;         // associated buffer of the canvas

    uint16_t line_alt1_step;     // step for line_alt1, typically  10m depending on canvas_h
    uint16_t line_alt2_step;     // step for line_alt2, typically  50m depending on canvas_h
    uint16_t line_alt3_step;     // step for line_alt3, typically 100m depending on canvas_h

    uint16_t last_index;      // stores the last displayed fc.history.index to update only on change.
);

static bool static_init = false;
static lv_draw_line_dsc_t line_alt1;     // for horizontal line1
static lv_draw_line_dsc_t line_alt2;     // for horizontal line2
static lv_draw_line_dsc_t line_alt3;     // for horizontal line3
static lv_draw_img_dsc_t glider_dsc;     // the image of the glider at the right side.

static void AltitudeGraph_init(lv_obj_t * base, widget_slot_t * slot)
{
	int t_h;

	if (!static_init)
	{
		lv_draw_line_dsc_init(&line_alt1);
		line_alt1.color = LV_COLOR_GRAY;
		line_alt1.width = 1;
		line_alt1.dash_width = 5;
		line_alt1.dash_gap = 7;

		lv_draw_line_dsc_init(&line_alt2);
		line_alt2.color = LV_COLOR_GRAY;
		line_alt2.width = 1;

		lv_draw_line_dsc_init(&line_alt3);
		line_alt3.color = LV_COLOR_WHITE;
		line_alt3.width = 2;

		lv_draw_img_dsc_init(&glider_dsc);

		static_init = true;
	}

	widget_create_base(base, slot);

	if (!widget_flag_is_set(slot, wf_label_hide))
	{
		widget_add_title(base, slot, slot->widget->name);
		t_h = lv_obj_get_height(slot->title);
	} else
		t_h = 0;

	int canvas_h = slot->h - t_h;

	local->canvas = lv_canvas_create(slot->obj, NULL);
	local->cbuf = malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(slot->w, canvas_h));
	lv_canvas_set_buffer(local->canvas, local->cbuf, slot->w, canvas_h, LV_IMG_CF_TRUE_COLOR);
	lv_obj_set_pos(local->canvas, 0, t_h);
	lv_obj_set_size(local->canvas, slot->w, canvas_h);

	uint8_t units = config_get_select(&config.units.altitude);
	switch (units)
	{
		case ALTITUDE_M:
			local->line_alt3_step = 100;
			local->line_alt2_step = 50;
			local->line_alt1_step = 10;
			break;
		case ALTITUDE_FT:
			local->line_alt3_step = 500;
			local->line_alt2_step = 100;
			local->line_alt1_step = 25;
			break;
	}
	DBG("canvas_h=%d line_alt1/2/3_step=%d/%d/%d", canvas_h, local->line_alt1_step, local->line_alt2_step, local->line_alt3_step);

	local->last_index = UINT16_MAX;
}

static void AltitudeGraph_stop(widget_slot_t * slot)
{
	free(local->cbuf);
	local->cbuf = NULL;
}

/**
 * Return the color associated with a given gain in altitude.
 *
 * @param gain gain/loss in m/s * 100
 *
 * @return the color associated with this.
 */
lv_color_t get_vario_color(int gain)
{
	lv_color_t c;

	if (gain >= 250 )
		c = lv_color_make(255, 0, 0);
	else if (gain >= 200 )
		c = lv_color_make(255, 100, 0);
	else if (gain >= 100 )
		c = lv_color_make(255, 200, 0);
	else if (gain >= 0 )
		c = lv_color_make(255, 255, 0);
	else if (gain >= -100 )
		c = lv_color_make(0, 255, 255);
	else if (gain >= -200 )
		c = lv_color_make(0, 150, 255);
	else
		c = lv_color_make(0, 0, 255);

	return c;
}

/**
 * Return the color associated with a given gain in altitude.
 * This implementation was taken from widget_thermal_assistant.
 *
 * @param gain gain/loss in m/s * 100
 *
 * @return the color associated with this.
 */
lv_color_t get_vario_color2(int gain)
{
	lv_color_t c = LV_COLOR_SILVER;

	if (gain < -60) {
		c = LV_COLOR_GRAY;
	} else if (gain < 0) {
		c = LV_COLOR_SILVER;
	} else if (gain < 50) {
		c = LV_COLOR_MAKE(0x11, 0xcc, 0x11);
	} else if (gain < 100) {
		c = LV_COLOR_MAKE(0x00, 0xAF, 0x00);
	} else if (gain < 150) {
		c = LV_COLOR_GREEN;
	} else if (gain < 220) {
		c = LV_COLOR_MAKE(0xFF, 0x6F, 0x00);
	} else if (gain >= 250) {
		c = LV_COLOR_MAKE(0xFF, 0x1A, 0x00);
	} else {
		c = LV_COLOR_SILVER;
	}

	return c;
}


static void AltitudeGraph_update(widget_slot_t * slot)
{
	int values_num = 0;
	int values_min = INT_MAX;
	int values_max = INT_MIN;
	uint16_t values[NUM_PLOTS];
	char horizontal_unit[10];

	if ( fc.history.index == local->last_index)
	{
		// Nothing has changed.
		return;
	}

	local->last_index = fc.history.index;

	/*
	 * [0] Collect values.
	 */

	uint8_t units = config_get_select(&config.units.altitude);
	format_altitude_units_2(horizontal_unit, units);

	int16_t i = GRAPH_HISTORY_STEP * 2; // skip last 2 history positions

	while (i < fc.history.size)
	{
		uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i) % FC_HISTORY_SIZE;
		fc_pos_history_t * pos = &fc.history.positions[index];

		bool valid = pos->flags & FC_POS_GNSS_3D;
		if ( valid )
		{
			values[values_num] = pos->gnss_alt;
			if (units == ALTITUDE_FT)
				values[values_num] *= FC_METER_TO_FEET;

			values_max = max(values[values_num], values_max);
			values_min = min(values[values_num], values_min);

			// DBG("index=%d, values_num=%d, values[values_num]=%d, values_min=%d, values_max=%d", index, values_num, values[values_num], values_min, values_max);

			values_num++;

			if ( values_num >= NUM_PLOTS ) break;
		}
		i += GRAPH_HISTORY_STEP;
	}

	int values_diff = values_max - values_min;
	DBG("values_num=%d, values_min=%d, values_max=%d, values_diff=%d", values_num, values_min, values_max, values_diff);

	int canvas_h = lv_obj_get_height(local->canvas);
	int canvas_w = lv_obj_get_width(local->canvas);
	int track_w = canvas_w - glider.header.w;        // the track area is smaller than canvas, as the glider is right of the track
	int track_h = canvas_h - glider.header.h;        // the track area is smaller than canvas, as the glider/2 must be above/below track.
	int track_off_y = glider.header.h / 2;           // this is the offset of the track area in the canvas to have room for glider icon.

	lv_point_t p[2];
	lv_point_t line_p[2];

	lv_canvas_fill_bg(local->canvas, LV_COLOR_BLACK, LV_OPA_COVER);

	if ( values_diff != 0 && values_num != 0)
	{

		int i = 0;

		/*
		 * [1] Draw horizonal (altitude) lines
		 */
		lv_draw_line_dsc_t *line_horiz_dsc;

		// Prepare dsc for label of horizontal lines (altitude):
		lv_draw_label_dsc_t label_dsc;
		lv_draw_label_dsc_init(&label_dsc);
		label_dsc.color = LV_COLOR_YELLOW;
		label_dsc.font = &lv_font_montserrat_12;

		// Prepare line start and end point for horizontal line over canvas:
		line_p[0].x = 0;
		line_p[1].x = canvas_w - 1;

		bool first_label = true;           // To find the first label
		bool label_drawn = false;          // have we drawn any label?
		lv_point_t label_pos;              // the position of the current label
		char label_buffer[20];             // the text shown on the label
		lv_point_t first_label_pos = {0};  // the position of the lowest label
		char first_label_buffer[20];       // the text of the lowest label

		for ( int y = (values_min/local->line_alt1_step + 1)*local->line_alt1_step; y < values_max ; y += local->line_alt1_step)
		{
			if ( y % local->line_alt1_step == 0 )
			{
				if ( y % local->line_alt3_step == 0 )      line_horiz_dsc = &line_alt3;
				else if ( y % local->line_alt2_step == 0 ) line_horiz_dsc = &line_alt2;
				else                                       line_horiz_dsc = &line_alt1;

				line_p[0].y = line_p[1].y = track_off_y + track_h - ((y - values_min) * track_h / values_diff);
				lv_canvas_draw_line(local->canvas, line_p, 2, line_horiz_dsc);

				label_pos.x = 10;
				label_pos.y = line_p[0].y - lv_font_montserrat_12.line_height + lv_font_montserrat_12.base_line - 1;
				sprintf(label_buffer, "%d %s", y, horizontal_unit);

				if ( y % local->line_alt2_step == 0 )
				{
					label_drawn = true;
					lv_canvas_draw_text(local->canvas, label_pos.x, label_pos.y, 100, &label_dsc, label_buffer, LV_LABEL_ALIGN_LEFT);
				} else {
					if ( first_label )
					{
						strcpy(first_label_buffer, label_buffer);
						first_label_pos = label_pos;
						first_label = false;
					}
				}
			}
		}  

		if ( !label_drawn && !first_label )
		{
			// This is a special case: we have not given any labels, so fallback to print label on lowest line
			lv_canvas_draw_text(local->canvas, first_label_pos.x, first_label_pos.y, 100, &label_dsc, first_label_buffer, LV_LABEL_ALIGN_LEFT);
		}

		/*
		 * [2] Draw track
		 */

		lv_draw_line_dsc_t line_dsc;

		lv_draw_line_dsc_init(&line_dsc);
		line_dsc.width = 2;

		p[0].x = track_w - 1;
		p[0].y = track_off_y + track_h - ((values[0] - values_min) * track_h / values_diff);

		lv_canvas_draw_img(local->canvas, p[0].x + 1, p[0].y - glider.header.h/2, &glider, &glider_dsc);

		for ( i = 1; i < values_num; i++ )
		{
			int value_diff;

			value_diff = values[i-1] - values[i];
			line_dsc.color = get_vario_color(value_diff * 100);

			p[1].x = track_w - i * track_w / values_num;
			p[1].y = track_off_y + track_h - ((values[i] - values_min) * track_h / values_diff);
			// DBG("i=%d pos->gnss_alt=%d x1/y1=%d/%d x2/y2=%d/%d", i, values[i], p[0].x, p[0].y, p[1].x, p[1].y);

			lv_canvas_draw_line(local->canvas, p, 2, &line_dsc);

			p[0] = p[1];
		}
	}
}


