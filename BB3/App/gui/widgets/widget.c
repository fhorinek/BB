/*
 * widget.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#include "widget.h"


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

        if (unit_obj != NULL)
            *unit_obj =  unit_label;
    }

    return value_obj;
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
	for (i = 0; i < NUMBER_OF_WIDGET_FONTS; i++)
	{
		_lv_txt_get_size(&size, ext->text, gui.styles.widget_fonts[i], letter_space, line_space, LV_COORD_MAX, LV_TXT_FLAG_EXPAND);
		if (size.x <= w && size.y <= h)
			break;
	}

	//set smallest as fallback
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, gui.styles.widget_fonts[i]);
}
