/*
 * widget.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#include "widget.h"

#include "gui/tasks/page/pages.h"

widget_flag_def_t widgets_flags[] = {
	{'L', LV_SYMBOL_EYE_CLOSE " Label hidden", LV_SYMBOL_EYE_OPEN " Label visible", NULL},
    {'U', LV_SYMBOL_EYE_CLOSE " Units hidden", LV_SYMBOL_EYE_OPEN " Units visible", NULL},
    {'D', LV_SYMBOL_EYE_OPEN " Decimals visible", LV_SYMBOL_EYE_CLOSE " Decimals hidden", NULL},
    {'A', "Alternative units", "Default units", NULL},
    {'V', "Avg. vario on climb", "Empty on climb", NULL},
    {'R', "North is up", "Adjust to heading", NULL},
    {'H', LV_SYMBOL_EYE_CLOSE " Hide icon", LV_SYMBOL_EYE_OPEN " Show icon", NULL},
};


void widget_create_base(lv_obj_t * base, widget_slot_t * slot)
{
    slot->obj = lv_obj_create(base, NULL);
    lv_obj_set_pos(slot->obj, slot->x, slot->y);
    lv_obj_set_size(slot->obj, slot->w, slot->h);
    lv_obj_add_style(slot->obj, LV_OBJ_PART_MAIN, &gui.styles.widget_box);
}

void widget_add_title(lv_obj_t * base, widget_slot_t * slot, char * title)
{
	lv_obj_t * title_label = lv_label_create(slot->obj, NULL);
	if (title == NULL)
	    title = slot->widget->short_name;

	lv_label_set_text(title_label, title);
	lv_label_set_long_mode(title_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_align(title_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_add_style(title_label, LV_LABEL_PART_MAIN, &gui.styles.widget_label);
    lv_obj_set_width(title_label, slot->w);
	lv_obj_align(title_label, slot->obj, LV_ALIGN_IN_TOP_MID, 0, 0);

	slot->title = title_label;
}

#define WIDGET_VALUE_MIN_HEIGHT     16

/*
 * Add a value with optional unit to a widget, e.g ground speed 25 km/h.
 *
 * @param base the GUI lv_obj_t where the value should be put into
 * @param slot the widget running in base where the value belongs to
 * @param unit a text placed under the value, e.g. "km/h" or NULL if no label is needed.
 * @param unit_obj receives the lv_obj_t of the unit label, so that it can
 *                 be used later to change the unit label. If this is NULL, then
 *                 nothing will be returned.
 *
 * @return the GUI lv_obj_t of the value object.
 */
lv_obj_t * widget_add_value(lv_obj_t * base, widget_slot_t * slot, char * unit, lv_obj_t ** unit_obj)
{
    lv_obj_t * unit_label = NULL;

    // Todo: parameter base seems to be unused and replaced by slot->obj. Remove "base" as parameter?
    if (unit != NULL)
    {
        unit_label = lv_label_create(slot->obj, NULL);
        lv_label_set_text(unit_label, unit);
        lv_label_set_long_mode(unit_label, LV_LABEL_LONG_EXPAND);
        lv_label_set_align(unit_label, LV_LABEL_ALIGN_CENTER);
        lv_obj_add_style(unit_label, LV_LABEL_PART_MAIN, &gui.styles.widget_unit);
        lv_obj_set_width(unit_label, slot->w);
        lv_obj_align(unit_label, slot->obj, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_obj_set_auto_realign(unit_label, true);
    }

    int16_t t_h = 0;
    int16_t u_h = 0;

    if (slot->title != NULL)
        t_h = lv_obj_get_height(slot->title);
    if (unit_label != NULL)
        u_h = lv_obj_get_height(unit_label);

    if (slot->h - WIDGET_VALUE_MIN_HEIGHT <= t_h + u_h)
    {
        if (u_h > 0)
        {
            u_h = 0;
            lv_obj_del(unit_label);
            unit_label = NULL;
        }

        if (slot->h - WIDGET_VALUE_MIN_HEIGHT <= t_h)
        {
            t_h = 0;
            lv_obj_del(slot->title);
            slot->title = NULL;
        }
    }

    lv_obj_t * value_cont = lv_obj_create(slot->obj, NULL);
    lv_obj_set_pos(value_cont, 0, t_h);
    lv_obj_set_size(value_cont, slot->w, slot->h - u_h - t_h);
    lv_obj_set_style_local_bg_opa(value_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    lv_obj_t * value_obj = lv_label_create(value_cont, NULL);
    lv_label_set_text(value_obj, "");
    lv_label_set_long_mode(value_obj, LV_LABEL_LONG_EXPAND);
    lv_label_set_align(value_obj, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(value_obj, value_cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(value_obj, true);

    if (unit_obj != NULL)
        *unit_obj = unit_label;

    return value_obj;
}

void widget_arrow_rotate_size(lv_obj_t * arrow, lv_point_t * points, int16_t angle, uint8_t s)
{
    // make sure, that angle is always between 0 and 359:
     if (angle < 0 || angle > 359)
         angle = (angle + 360) % 360;

     uint8_t mx = lv_obj_get_width(arrow) / 2;
     uint8_t my = lv_obj_get_height(arrow) / 2;
     float fsin = table_sin(angle);
     float fcos = table_cos(angle);

     points[0].x = mx + fsin * s / 3;
     points[0].y = my + fcos * s / 3;
     points[2].x = mx - fsin * s / 5;
     points[2].y = my - fcos * s / 5;

     fsin = table_sin(angle + 25);
     fcos = table_cos(angle + 25);
     points[1].x = mx - fsin * s / 3;
     points[1].y = my - fcos * s / 3;

     fsin = table_sin(angle + 335);
     fcos = table_cos(angle + 335);
     points[3].x = mx - fsin * s / 3;
     points[3].y = my - fcos * s / 3;

     points[4].x = points[0].x;
     points[4].y = points[0].y;

     lv_obj_invalidate(arrow);
}


void widget_arrow_rotate(lv_obj_t * arrow, lv_point_t * points, int16_t angle)
{
    int16_t w = lv_obj_get_width(arrow);
    int16_t h = lv_obj_get_height(arrow);
    uint8_t s = min(w, h);
    widget_arrow_rotate_size(arrow, points, angle,s);
}

/**
 * Add an arrow to the widget with a possible unit.
 *
 * @param base the base widget (currently unused)
 * @param slot the widget to which this arrow is added
 * @param points a pointer to lv_point_t[WIDGET_ARROW_POINTS]
 *               allocated by caller. This array receives the points of the
 *               arrow and allows the caller to modify (rotate) them.
 * @param unit a text placed under the value, e.g. "km/h" or NULL if no label is needed.
 * @param unit_obj receives the lv_obj_t of the unit label, so that it can
 *                 be used later to change the unit label. If this is NULL, then
 *                 nothing will be returned.
 *
 * @return the GUI lv_obj_t of the arrow object.
 */
lv_obj_t * widget_add_arrow(lv_obj_t * base, widget_slot_t * slot, lv_point_t * points, char * unit, lv_obj_t ** unit_obj)
{
	// Todo: remove base, as it seems unused?
	lv_obj_t * unit_label = NULL;

    if (unit != NULL)
    {
        unit_label = lv_label_create(slot->obj, NULL);
        lv_label_set_text(unit_label, unit);
        lv_label_set_long_mode(unit_label, LV_LABEL_LONG_EXPAND);
        lv_label_set_align(unit_label, LV_LABEL_ALIGN_CENTER);
        lv_obj_add_style(unit_label, LV_LABEL_PART_MAIN, &gui.styles.widget_unit);
        lv_obj_align(unit_label, slot->obj, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_obj_set_auto_realign(unit_label, true);
    }

    int16_t t_h = 0;
    int16_t u_h = 0;

    if (slot->title != NULL)
        t_h = lv_obj_get_height(slot->title);
    if (unit_label != NULL)
        u_h = lv_obj_get_height(unit_label);

    if (slot->h - WIDGET_VALUE_MIN_HEIGHT <= t_h + u_h)
    {
        if (u_h > 0)
        {
            u_h = 0;
            lv_obj_del(unit_label);
            unit_label = NULL;
        }

        if (slot->h - WIDGET_VALUE_MIN_HEIGHT <= t_h)
        {
            t_h = 0;
            lv_obj_del(slot->title);
            slot->title = NULL;
        }
    }

    lv_obj_t * arrow_obj = lv_line_create(slot->obj, NULL);
    lv_obj_set_pos(arrow_obj, 0, t_h);
    lv_obj_set_size(arrow_obj, slot->w, slot->h - u_h - t_h);
    lv_line_set_auto_size(arrow_obj, false);
    memset(points, 0, sizeof(lv_point_t) * WIDGET_ARROW_POINTS);
    lv_line_set_points(arrow_obj, points, WIDGET_ARROW_POINTS);


    if (unit_obj != NULL)
        *unit_obj = unit_label;

    return arrow_obj;
}

void widget_update_font_size_box(lv_obj_t * label, lv_coord_t w, lv_coord_t h)
{
	static uint8_t font_size_cache_x_number[NUMBER_OF_WIDGET_FONTS] = {0};
	static uint8_t font_size_cache_x_char[NUMBER_OF_WIDGET_FONTS] = {0};
    static uint8_t font_size_cache_x_sign[NUMBER_OF_WIDGET_FONTS] = {0};
    static uint8_t font_size_cache_x_dot[NUMBER_OF_WIDGET_FONTS] = {0};
	static uint8_t font_size_cache_x_symbol[NUMBER_OF_WIDGET_FONTS] = {0};
	static uint8_t font_size_cache_y[NUMBER_OF_WIDGET_FONTS] = {0};

	lv_label_ext_t * ext = lv_obj_get_ext_attr(label);

	if (ext->text == NULL)
		return;

    uint8_t len = strlen(ext->text);
    uint8_t nums = 0;
    uint8_t signs = 0;
    uint8_t dots = 0;
    uint8_t chars = 0;
    uint8_t symbol = 0;
    uint8_t lines = 1;

    if (strcmp(ext->text, "---") == 0)
    {
        signs = 6;
    }
    else
    {
        for (uint8_t j = 0; j < len; j++)
        {
            char c = ext->text[j];
            if (c == '\n')
                lines++;
            else if (ISDIGIT(c))
                nums++;
            else if (c == '.' || c == ',' || c == ':'  || c == ' ')
                dots++;
            else if (c == '-' || c == '+')
                signs++;
            else if (ISALPHA(c))
                chars++;
            else
                symbol++;
        }
    }

	uint8_t i = 0;
	if (chars || symbol)
	    i = FONT_WITH_TEXTS;

	for (; i < NUMBER_OF_WIDGET_FONTS - 1; i++)
	{
		if (font_size_cache_x_number[i] == 0)
		{
		    lv_point_t size;

			_lv_txt_get_size(&size, "0", gui.styles.widget_fonts[i], 0, 0, LV_COORD_MAX, LV_TXT_FLAG_EXPAND);
			font_size_cache_x_number[i] = size.x;
            _lv_txt_get_size(&size, "-", gui.styles.widget_fonts[i], 0, 0, LV_COORD_MAX, LV_TXT_FLAG_EXPAND);
            font_size_cache_x_sign[i] = size.x;
            _lv_txt_get_size(&size, ".", gui.styles.widget_fonts[i], 0, 0, LV_COORD_MAX, LV_TXT_FLAG_EXPAND);
            font_size_cache_x_dot[i] = size.x;
			_lv_txt_get_size(&size, "%", gui.styles.widget_fonts[i], 0, 0, LV_COORD_MAX, LV_TXT_FLAG_EXPAND);
			font_size_cache_x_symbol[i] = size.x;
			_lv_txt_get_size(&size, "X", gui.styles.widget_fonts[i], 0, 0, LV_COORD_MAX, LV_TXT_FLAG_EXPAND);
			font_size_cache_x_char[i] = size.x;
            _lv_txt_get_size(&size, "0", gui.styles.widget_fonts[i], 0, 0, LV_COORD_MAX, LV_TXT_FLAG_EXPAND);
			font_size_cache_y[i] = size.y - gui.styles.widget_fonts[i]->base_line;

//		    static uint16_t y_start = 20;
//		    static uint16_t x_start = 20;
//
//		    lv_obj_t * box = lv_obj_create(lv_layer_sys(), NULL);
//		    lv_obj_set_pos(box, x_start, y_start);
//		    lv_obj_set_size(box, font_size_cache_x_number[i], font_size_cache_y[i]);
//            lv_obj_set_style_local_border_width(box, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
//            lv_obj_set_style_local_border_color(box, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
//		    y_start += font_size_cache_y[i] + 5;
//
//		    if (i == 3)
//		    {
//		        y_start = 20;
//		        x_start = 120;
//		    }
//
//		    lv_obj_t * lab = lv_label_create(box, NULL);
//            lv_obj_set_style_local_text_font(lab, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, gui.styles.widget_fonts[i]);
//		    lv_label_set_long_mode(lab, LV_LABEL_LONG_EXPAND);
//            lv_label_set_text_fmt(lab, "%u", i);
//		    lv_obj_align(lab, box, LV_ALIGN_CENTER, 0, 0);
		}
		uint16_t x = 0;
        x += nums * font_size_cache_x_number[i];
        x += signs * font_size_cache_x_sign[i];
        x += dots * font_size_cache_x_dot[i];
        x += symbol * font_size_cache_x_symbol[i];
        x += chars * font_size_cache_x_char[i];

		uint16_t y = lines * font_size_cache_y[i] + (gui.styles.widget_fonts[i]->base_line * (lines - 1));

		if (x < w && y < h)
			break;
	}
//	INFO("fs %u", i);

	//set smallest as fallback
	if (lv_obj_get_style_text_font(label, LV_LABEL_PART_MAIN) != gui.styles.widget_fonts[i])
	{
		lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, gui.styles.widget_fonts[i]);
	}
}

void widget_update_font_size(lv_obj_t * label)
{
    lv_obj_t * area = lv_obj_get_parent(label);
    lv_coord_t w = lv_obj_get_width(area);
    lv_coord_t h = lv_obj_get_height(area);

    widget_update_font_size_box(label, w, h);
}

static void widget_opa_anim(lv_obj_t * obj, lv_anim_value_t v)
{
    lv_obj_set_style_local_opa_scale(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, v);
}

lv_obj_t * widget_create_edit_overlay(char * title, char * message)
{
    lv_obj_t * base = lv_obj_create(lv_layer_sys(), NULL);
    lv_obj_set_pos(base, 0, 0);
    lv_obj_set_size(base, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_local_bg_color(base, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_BLACK);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, base);
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_90);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)widget_opa_anim);
    lv_anim_start(&a);

    lv_obj_t * cont = lv_cont_create(base, NULL);
    lv_cont_set_layout(cont, LV_LAYOUT_COLUMN_MID);
    lv_obj_set_auto_realign(cont, true);
    lv_obj_align_origo(cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_cont_set_fit(cont, LV_FIT_TIGHT);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    if (title != NULL)
    {
        lv_obj_t * title_label = lv_label_create(cont, NULL);
        lv_label_set_align(title_label, LV_LABEL_ALIGN_CENTER);
        lv_label_set_long_mode(title_label, LV_LABEL_LONG_BREAK);
        lv_obj_set_width(title_label, (LV_HOR_RES * 3) / 4);
        lv_obj_set_style_local_text_font(title_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_22);
        lv_label_set_text(title_label, title);
    }

    if (message != NULL)
    {
        lv_obj_t * text_label = lv_label_create(cont, NULL);
        lv_label_set_align(text_label, LV_LABEL_ALIGN_CENTER);
        lv_label_set_long_mode(text_label, LV_LABEL_LONG_BREAK);
        lv_obj_set_width(text_label, (LV_HOR_RES * 3) / 4);
        lv_obj_set_style_local_pad_top(text_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 10);
        lv_label_set_text(text_label, message);
    }

    pages_lock_widget();

    return base;
}

lv_obj_t * widget_edit_overlay_get_base(lv_obj_t * edit)
{
	return lv_obj_get_child(edit, NULL);
}

void widget_reset_edit_overlay_timer()
{
	pages_lock_reset();
}

void widget_destroy_edit_overlay(lv_obj_t * base)
{
    lv_obj_del_async(base);

    pages_unlock_widget();
}

static bool static_init = false;
static lv_draw_line_dsc_t line_alt1;     // for horizontal line1
static lv_draw_line_dsc_t line_alt2;     // for horizontal line2
static lv_draw_line_dsc_t line_alt3;     // for horizontal line3
static lv_draw_img_dsc_t glider_dsc;     // the image of the glider at the right side.
static lv_draw_label_dsc_t label_dsc;

/**
 * Add a timed graph to the widget. Prior to calling this method,
 * graph->w and graph->h must be set to the size of the graph.
 *
 * The caller is also responsible to set graph->line_alt1_step,
 * graph->line_alt2_step, graph->line_alt3_step. They descibe, at what value
 * steps the Y-Axes should get a horiziontal line with values.
 * For alitudes, this could be "10/50/100" which will draw altitude lines
 * every 10, 50 and 100m of height.
 *
 * The caller must also set graph->line_label_format to a printf compatible
 * format string used to print the description of the axes. For altitudes
 * this could be "%d m" or something else, depending on selected unit.
 *
 * @param base the base widget (currently unused)
 * @param slot the widget to which this arrow is added
 * @param graph a pointer to a caller allocated graph_t holding all
 *              necessary information about the graph.
 */
void widget_add_graph(lv_obj_t * base, widget_slot_t * slot, graph_t *graph)
{
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

		// Prepare dsc for label of horizontal lines (altitude):
		lv_draw_label_dsc_init(&label_dsc);
		label_dsc.color = LV_COLOR_YELLOW;
		label_dsc.font = &lv_font_montserrat_12;

		static_init = true;
	}

	graph->canvas = lv_canvas_create(slot->obj, NULL);
	graph->cbuf = malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(graph->w, graph->h));
	lv_canvas_set_buffer(graph->canvas, graph->cbuf, graph->w, graph->h, LV_IMG_CF_TRUE_COLOR);
	lv_obj_set_size(graph->canvas, graph->w, graph->h);
}

/**
 * Update the graph with new values. The parameter values_min/max are used to
 * describe the Y-axis. For floating axes this could be the minimum/maximum value
 * found in "values". For fixed axes this could be something like "0" and "100".
 *
 * @param graph a pointer to the graph to update.
 * @param values an array containing all values to be displayed.
 * @param values_num indicated, how many values are inside array "values".
 * @param values_min the minimum value to be found in "values".
 * @param values_max the maximum value to be found in "values".
 */
void widget_update_graph(widget_slot_t *slot, graph_t *graph, float values[], int values_num, float values_min, float values_max)
{
    float values_diff = values_max - values_min;
    DBG("widget_update_graph: values_num=%d, values_min=%f, values_max=%f, values_diff=%f", values_num, values_min, values_max, values_diff);

    int canvas_h = lv_obj_get_height(graph->canvas);
    int canvas_w = lv_obj_get_width(graph->canvas);
    int track_w = canvas_w - img_glider_icon.header.w; // the track area is smaller than canvas, as the glider is right of the track
    int track_h = canvas_h - img_glider_icon.header.h; // the track area is smaller than canvas, as the glider/2 must be above/below track.
    int track_off_y = img_glider_icon.header.h / 2; // this is the offset of the track area in the canvas to have room for glider icon.

    if (!widget_flag_is_set(slot, wf_hide_icon))
    {
        track_w = canvas_w - img_glider_icon.header.w; // the track area is smaller than canvas, as the glider is right of the track
        track_h = canvas_h - img_glider_icon.header.h; // the track area is smaller than canvas, as the glider/2 must be above/below track.
        track_off_y = img_glider_icon.header.h / 2; // this is the offset of the track area in the canvas to have room for glider icon.
    }
    else
    {
        track_w = canvas_w; // the track area is smaller than canvas, as the glider is right of the track
        track_h = canvas_h; // the track area is smaller than canvas, as the glider/2 must be above/below track.
        track_off_y = 0; // this is the offset of the track area in the canvas to have room for glider icon.
    }

    lv_point_t p[2];
    lv_point_t line_p[2];

    lv_canvas_fill_bg(graph->canvas, LV_COLOR_BLACK, LV_OPA_COVER);

    if (values_diff > 0.1 && values_num > 1)
    {
        int i = 0;

        /*
         * [1] Draw horizonal (altitude) lines
         */
        lv_draw_line_dsc_t *line_horiz_dsc;

        // Prepare line start and end point for horizontal line over canvas:
        line_p[0].x = 0;
        line_p[1].x = canvas_w - 1;

        bool first_label = true;           // To find the first label
        bool label_drawn = false;          // have we drawn any label?
        lv_point_t label_pos;              // the position of the current label
        char label_buffer[20];             // the text shown on the label
        lv_point_t first_label_pos = { 0 };  // the position of the lowest label
        char first_label_buffer[20];       // the text of the lowest label

        for (int y = ((int) values_min / graph->line_alt1_step + 1) * graph->line_alt1_step;
                y < values_max; y += graph->line_alt1_step)
        {
            if (y % graph->line_alt3_step == 0)
                line_horiz_dsc = &line_alt3;
            else if (y % graph->line_alt2_step == 0)
                line_horiz_dsc = &line_alt2;
            else
                line_horiz_dsc = &line_alt1;

            line_p[0].y = line_p[1].y = track_off_y + track_h - ((y - values_min) * track_h / values_diff);
            lv_canvas_draw_line(graph->canvas, line_p, 2, line_horiz_dsc);

            label_pos.x = 10;
            label_pos.y = line_p[0].y - lv_font_montserrat_12.line_height + lv_font_montserrat_12.base_line - 1;
            sprintf(label_buffer, graph->line_label_format, y);

            if (y % graph->line_alt2_step == 0)
            {
                label_drawn = true;
                lv_canvas_draw_text(graph->canvas, label_pos.x, label_pos.y, 100, &label_dsc, label_buffer, LV_LABEL_ALIGN_LEFT);
            }
            else
            {
                if (first_label)
                {
                    strcpy(first_label_buffer, label_buffer);
                    first_label_pos = label_pos;
                    first_label = false;
                }
            }
        }

        if (!label_drawn && !first_label)
        {
            // This is a special case: we have not given any labels, so fallback to print label on lowest line
            lv_canvas_draw_text(graph->canvas, first_label_pos.x, first_label_pos.y, 100, &label_dsc, first_label_buffer, LV_LABEL_ALIGN_LEFT);
        }

        /*
         * [2] Draw track
         */

        lv_draw_line_dsc_t line_dsc;

        lv_draw_line_dsc_init(&line_dsc);
        line_dsc.width = 2;

        p[0].x = track_w - 1;
        p[0].y = track_off_y + track_h - ((values[0] - values_min) * track_h / values_diff);

        if (!widget_flag_is_set(slot, wf_hide_icon))
            lv_canvas_draw_img(graph->canvas, p[0].x + 1, p[0].y - img_glider_icon.header.h / 2, &img_glider_icon, &glider_dsc);

        for (i = 1; i < values_num; i++)
        {
            int value_diff;

            value_diff = values[i - 1] - values[i];
            line_dsc.color = get_vario_color(value_diff * 100);

            p[1].x = track_w - i * track_w / values_num;
            p[1].y = track_off_y + track_h - ((values[i] - values_min) * track_h / values_diff);
            // DBG("i=%d values[i]=%f x1/y1=%d/%d x2/y2=%d/%d", i, values[i], p[0].x, p[0].y, p[1].x, p[1].y);

            lv_canvas_draw_line(graph->canvas, p, 2, &line_dsc);

            p[0] = p[1];
        }
    }
}

/**
 * Remove the graph from the widget and free all resources.
 *
 * @param graph a pointer to the graph to be free'd.
 */
void widget_remove_graph(graph_t * graph)
{
	free(graph->cbuf);
	graph->cbuf = NULL;
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


