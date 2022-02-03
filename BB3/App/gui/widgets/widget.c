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
