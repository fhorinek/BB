/*
 * widget.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#include "widget.h"

#include "gui/tasks/page/pages.h"

void widget_create_base(lv_obj_t * base, widget_slot_t * slot)
{
    slot->obj = lv_obj_create(base, NULL);
    lv_obj_set_pos(slot->obj, slot->x, slot->y);
    lv_obj_set_size(slot->obj, slot->w, slot->h);
    lv_obj_add_style(slot->obj, LV_OBJ_PART_MAIN, &gui.styles.widget_box);
}

lv_obj_t * widget_add_title(lv_obj_t * base, widget_slot_t * slot, char * title)
{
	lv_obj_t * title_label = lv_label_create(slot->obj, NULL);
	if (title == NULL)
	    title = slot->widget->short_name;

	lv_label_set_text(title_label, title);
	lv_label_set_long_mode(title_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_align(title_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_add_style(title_label, LV_LABEL_PART_MAIN, &gui.styles.widget_label);
	lv_obj_align(title_label, slot->obj, LV_ALIGN_IN_TOP_MID, 0, 0);

	return title_label;
}

lv_obj_t * widget_add_value(lv_obj_t * base, widget_slot_t * slot, char * unit, lv_obj_t ** unit_obj)
{
    lv_obj_t * value_obj = lv_label_create(slot->obj, NULL);
    lv_label_set_text(value_obj, "---");
    lv_label_set_long_mode(value_obj, LV_LABEL_LONG_EXPAND);
    lv_label_set_align(value_obj, LV_LABEL_ALIGN_CENTER);
//    lv_obj_add_style(title_label, LV_LABEL_PART_MAIN, &gui.styles.widget_value);
    lv_obj_align(value_obj, slot->obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(value_obj, true);

    if (unit != NULL)
    {
        lv_obj_t * unit_label = lv_label_create(slot->obj, NULL);
        lv_label_set_text(unit_label, unit);
        lv_label_set_long_mode(unit_label, LV_LABEL_LONG_EXPAND);
        lv_label_set_align(unit_label, LV_LABEL_ALIGN_CENTER);
        lv_obj_add_style(unit_label, LV_LABEL_PART_MAIN, &gui.styles.widget_unit);
        lv_obj_align(unit_label, slot->obj, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_obj_set_auto_realign(unit_label, true);

        if (unit_obj != NULL)
            *unit_obj = unit_label;
    }

    return value_obj;
}

void widget_arrow_rotate(lv_obj_t * arrow, lv_point_t * points, int16_t angle)
{
    // make sure, that angle is always between 0 and 359:
     if (angle < 0 || angle > 359)
         angle = (angle % 360 + 360) % 360;

     int16_t w = lv_obj_get_width(arrow);
     int16_t h = lv_obj_get_height(arrow);
     uint8_t s = (min(w, h) * 2) / 3;

     uint8_t mx = w / 2;
     uint8_t my = h / 2;
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

lv_obj_t * widget_add_arrow(lv_obj_t * base, widget_slot_t * slot, lv_point_t * points, char * unit, lv_obj_t ** unit_obj)
{
    lv_obj_t * arrow_obj = lv_line_create(slot->obj, NULL);
    lv_obj_set_pos(arrow_obj, 0, 0);
    lv_obj_set_size(arrow_obj, slot->w, slot->h);
    lv_line_set_auto_size(arrow_obj, false);
    memset(points, 0, sizeof(lv_point_t) * WIDGET_ARROW_POINTS);
    lv_line_set_points(arrow_obj, points, WIDGET_ARROW_POINTS);

    if (unit != NULL)
    {
        lv_obj_t * unit_label = lv_label_create(slot->obj, NULL);
        lv_label_set_text(unit_label, unit);
        lv_label_set_long_mode(unit_label, LV_LABEL_LONG_EXPAND);
        lv_label_set_align(unit_label, LV_LABEL_ALIGN_CENTER);
        lv_obj_add_style(unit_label, LV_LABEL_PART_MAIN, &gui.styles.widget_unit);
        lv_obj_align(unit_label, slot->obj, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_obj_set_auto_realign(unit_label, true);

        if (unit_obj != NULL)
            *unit_obj = unit_label;
    }

    return arrow_obj;
}


void widget_update_font_size(lv_obj_t * label, lv_obj_t * area)
{
	lv_style_int_t line_space = lv_obj_get_style_text_line_space(label, LV_LABEL_PART_MAIN);
	lv_style_int_t letter_space = lv_obj_get_style_text_letter_space(label, LV_LABEL_PART_MAIN);
	lv_coord_t w = lv_obj_get_width_fit(area);
	lv_coord_t h = lv_obj_get_height_fit(area);
	lv_point_t size;

	lv_label_ext_t * ext = lv_obj_get_ext_attr(label);

	if (ext->text == NULL)
		return;

	uint8_t i;
	uint8_t len = strlen(ext->text) - 1;
	for (i = 0; i < NUMBER_OF_WIDGET_FONTS; i++)
	{
		_lv_txt_get_size(&size, "0", gui.styles.widget_fonts[i], letter_space, line_space, LV_COORD_MAX, LV_TXT_FLAG_EXPAND);
		size.x *= len;
		if (size.x <= w && size.y <= h)
			break;
	}

	//set smallest as fallback
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, gui.styles.widget_fonts[i]);
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

void widget_destroy_edit_overlay(lv_obj_t * base)
{
    lv_obj_del_async(base);

    pages_unlock_widget();
}
