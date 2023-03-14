
#include "gui/widgets/widget.h"
#include "drivers/power/pwr_mng.h"
#include "drivers/tft/tft.h"
#include "fc/agl.h"

#include "gui/line.h"
#include "etc/geo_calc.h"

// How many airspaces can we show -> how many rectangles?
#define AIRSPACE_NUM 10

// How far do we display in front of pilot in m?
#define MAX_DISTANCE_FORWARD (10 * 1000)
// How far do we display behind pilot in m?
#define MAX_DISTANCE_BACKWARD (2 * 1000)

#define MAX_DISTANCE (MAX_DISTANCE_FORWARD+MAX_DISTANCE_BACKWARD)

// What is the distance between the distance label and arrow?
#define LABEL_SPACE 7

REGISTER_WIDGET_IU
(
		AirspaceSide,
		_i("Airspace - side"),
		WIDGET_MIN_W,
		WIDGET_MIN_H,
		_b(wf_label_hide),

		// the glider of the pilot:
		lv_obj_t *glider;

	    // the airspaces, their label and text:
		lv_obj_t *as[AIRSPACE_NUM];
		lv_obj_t *as_label[AIRSPACE_NUM];
		char as_label_text[AIRSPACE_NUM][AIRSPACE_NAME_LEN*2];

		// The arrow line to show MAX_DISTANCE_FORWARD and its label:
		lv_obj_t *line1, *line2;
		lv_point_t line1_points[5];
		lv_point_t line2_points[5];
		lv_obj_t *line_label;
		char line_label_text[10];

		// the above and middle altitude label at the left side.
		lv_obj_t *alt1_label;
		char alt1_text[10];
		lv_obj_t *alt2_label;
		char alt2_text[10];

		// The ground level lines (vertical)
		lv_obj_t *gl_lines[TFT_WIDTH];
		lv_point_t gl_points[TFT_WIDTH][2];

		// When did we last update?
		uint32_t last_updated;
);


static void AirspaceSide_init(lv_obj_t * base, widget_slot_t * slot)
{
  local->last_updated = 0;
  
	widget_create_base(base, slot);

	if (!widget_flag_is_set(slot, wf_label_hide))
		widget_add_title(base, slot, NULL);

	static lv_style_t dashed_style;
    lv_style_init(&dashed_style);
    lv_style_set_line_dash_gap(&dashed_style, LV_STATE_DEFAULT, 5);
    lv_style_set_line_dash_width(&dashed_style, LV_STATE_DEFAULT, 10);

  	lv_obj_set_style_local_bg_color(slot->obj, LV_OBJ_PART_MAIN,
			LV_STATE_DEFAULT, lv_color_make(150, 150, 255));

	for (int i = 0; i < AIRSPACE_NUM; i++)
	{
		local->as[i] = lv_obj_create(slot->obj, NULL);
		lv_obj_set_style_local_border_width(local->as[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_border_color(local->as[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_obj_set_hidden(local->as[i], true);

		local->as_label[i] = lv_label_create(local->as[i], NULL);
		lv_obj_align(local->as_label[i], local->as[i], LV_ALIGN_CENTER, 0, 0);
		lv_obj_add_style(local->as_label[i], LV_LABEL_PART_MAIN, &gui.styles.widget_label);
	}

	local->alt1_label = lv_label_create(slot->obj, NULL);
	lv_obj_add_style(local->alt1_label, LV_LABEL_PART_MAIN, &gui.styles.widget_label);
	lv_obj_set_style_local_text_color(local->alt1_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	local->alt2_label = lv_label_create(slot->obj, local->alt1_label);

	lv_coord_t x = slot->w * MAX_DISTANCE_BACKWARD / MAX_DISTANCE;
	lv_coord_t y = slot->h - 10;
	lv_point_t p1, p2;

    bzero(local->gl_points, sizeof(local->gl_points));
    for (int x = 0; x < slot->w; x++)
    {
    	local->gl_lines[x] = lv_line_create(slot->obj, NULL);
        lv_line_set_points(local->gl_lines[x], local->gl_points[x], 2);
        lv_obj_set_style_local_line_color(local->gl_lines[x], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    }

    // Distance arrow showing how far we see
    lv_color_t arrow_color = LV_COLOR_BLACK;
    local->line_label = lv_label_create(slot->obj, NULL);
    format_distance_with_units2(local->line_label_text, (float)MAX_DISTANCE_FORWARD );
    lv_label_set_text(local->line_label,local->line_label_text);
    lv_obj_align(local->line_label, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_align(local->line_label, LV_ALIGN_CENTER);
    lv_obj_set_style_local_text_color(local->line_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, arrow_color);

    lv_coord_t lw = lv_obj_get_width(local->line_label) + LABEL_SPACE*2;
    lv_coord_t lh = lv_obj_get_height(local->line_label);

    p1.x = x; p1.y = y;
	p2.x = x + (slot->w - x - lw)/2; p2.y = y;

	compute_line_arrow_points(p2, p1, local->line1_points);
    local->line1 = lv_line_create(slot->obj, NULL);
    lv_line_set_points(local->line1,local->line1_points, LINE_ARROW_NUM_POINTS);
    lv_obj_add_style(local->line1, LV_LINE_PART_MAIN, &dashed_style);
    lv_obj_set_style_local_line_color(local->line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, arrow_color);
    lv_obj_set_pos(local->line_label, p2.x + LABEL_SPACE, y - lh/2);

    p1 = p2;

    p1.x += lw;
    p2.x = slot->w;
    p2.y = y;
	compute_line_arrow_points(p1, p2, local->line2_points);
    local->line2 = lv_line_create(slot->obj, local->line1);
    lv_line_set_points(local->line2,local->line2_points, LINE_ARROW_NUM_POINTS);

	local->glider = lv_img_create(slot->obj, NULL);
	lv_img_set_src(local->glider, &img_glider_sideview);
    lv_obj_set_style_local_image_recolor(local->glider, LV_IMG_PART_MAIN,
            LV_STATE_DEFAULT, LV_COLOR_BLACK);
}

static void AirspaceSide_update(widget_slot_t * slot)
{
	int16_t max_height;
	lv_point_t glider_pos;
	lv_coord_t y;

	max_height = (((int16_t)fc.fused.altitude1 + 300) / 500 + 1) * 500;

	// Show glider:
	glider_pos.x = slot->w * MAX_DISTANCE_BACKWARD / MAX_DISTANCE;
	glider_pos.y = slot->h - slot->h * (int16_t)fc.fused.altitude1 / max_height;
	lv_obj_set_pos(local->glider, glider_pos.x - img_glider_sideview.header.w/2, glider_pos.y - img_glider_sideview.header.h);

	// Show altitude labels:
	format_altitude_with_units(local->alt1_text, max_height);
	lv_obj_set_pos(local->alt1_label, 0, 0);
	lv_label_set_text(local->alt1_label, local->alt1_text);
	if (abs(slot->h/2 - glider_pos.y) < 20)
	{
		// Not enough room
		lv_obj_set_pos(local->alt2_label, 0, slot->h/2);
		lv_label_set_text(local->alt2_label, "");
	}
	else
	{
		format_altitude_with_units(local->alt2_text, max_height/2);
		lv_obj_set_pos(local->alt2_label, 0, slot->h/2);
		lv_label_set_text(local->alt2_label, local->alt2_text);
	}

	// Is there anything available and has changed this last redraw?
	if (!fc.airspaces.valid || !fc.airspaces.near.valid || local->last_updated == fc.airspaces.near.last_updated)
		return;

	local->last_updated = fc.airspaces.near.last_updated;
	
	y = slot->h - slot->h * (int16_t)fc.fused.altitude1 / max_height;

	int as_count = 0;
	for (int i = 0; i < fc.airspaces.near.num; i++)
	{
		airspace_near_t *asn;
		airspace_record_t *as;

		asn = &fc.airspaces.near.asn[i];
		as = asn->as;

		// Check, if lower border of airspace is too heigh to be seen
		if (as->floor > max_height)
			continue;

		lv_coord_t as_floor_y, as_ceil_y;

		as_floor_y = slot->h - slot->h * (int16_t)as->floor / max_height;
		as_ceil_y  = slot->h - slot->h * (int16_t)as->ceil / max_height;
		if (as_ceil_y < 0) as_ceil_y = 0;

		for (int j = 0; j < AIRSPACE_NEAR_DISTANCE_NUM; j+=2)
		{
			int16_t distance_left = asn->distances[j] / 100;      // cm to m
			int16_t distance_right = asn->distances[j+1] / 100;      // cm to m
			if (distance_left == 0 || distance_right == 0) break;

			lv_coord_t as_x1 = glider_pos.x + slot->w * distance_left / MAX_DISTANCE;
			as_x1 = max(0, as_x1);
			lv_coord_t as_x2 = glider_pos.x + slot->w * distance_right / MAX_DISTANCE;
			as_x2 = min(slot->w, as_x2);
			if (abs(as_x1 - as_x2) < 3) continue;

			lv_obj_set_hidden(local->as[as_count], false);
			lv_obj_set_pos(local->as[as_count], as_x1, as_ceil_y);
			lv_obj_set_size(local->as[as_count], as_x2-as_x1, as_floor_y-as_ceil_y);
			lv_obj_set_style_local_bg_color(local->as[as_count], LV_OBJ_PART_MAIN,
					LV_STATE_DEFAULT, fc.airspaces.near.asn[i].as->brush);
            lv_obj_set_style_local_border_color(local->as[as_count], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, as->pen);
            lv_obj_set_style_local_border_opa(local->as[as_count], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);

			sprintf(local->as_label_text[as_count], "%s\n%s", airspace_class_name(as->airspace_class), as->name.ptr);
			lv_label_set_text(local->as_label[as_count], local->as_label_text[as_count]);

			as_count++;
			if ( as_count >= AIRSPACE_NUM)
			{
				// set outer index to end the outer loop
				i = fc.airspaces.near.num;

				// break the inner loop
				break;
			}
		}
	}

	// Set remaining (unused) airspace objects invisible
	for ( ; as_count < AIRSPACE_NUM; as_count++)
	{
		lv_obj_set_hidden(local->as[as_count], true);
	}

	// draw ground level
	gnss_pos_t pos;
	float step;

	geo_destination(fc.gnss.latitude, fc.gnss.longitude, fc.gnss.heading, -(float)MAX_DISTANCE_BACKWARD / 1000, &pos.latitude, &pos.longitude);
	step = ((float)MAX_DISTANCE / 1000) / slot->w;
	
    for (int x = 0; x < slot->w; x++)
    {
    	int16_t gl = agl_get_alt(pos.latitude, pos.longitude, false);

    	local->gl_points[x][0].x = x;
    	local->gl_points[x][0].y = slot->h - 1;
    	local->gl_points[x][1].x = x;
    	local->gl_points[x][1].y = slot->h - slot->h * gl / max_height;

        lv_line_set_points(local->gl_lines[x], local->gl_points[x], 2);

    	geo_destination(pos.latitude, pos.longitude, fc.gnss.heading, step, &pos.latitude, &pos.longitude);
    }
}


