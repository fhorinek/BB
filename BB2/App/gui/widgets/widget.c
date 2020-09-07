/*
 * widget.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#include "widget.h"


void widget_title(lv_obj_t * base, widget_slot_t * slot, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	slot->obj = lv_cont_create(base, NULL);
	lv_obj_set_pos(slot->obj, x, y);
	lv_obj_set_size(slot->obj, w, h);
	lv_cont_set_layout(slot->obj, LV_LAYOUT_PRETTY_MID);
	lv_obj_add_style(slot->obj, LV_LABEL_PART_MAIN, &gui.styles.widget_box);

	lv_obj_t * label = lv_label_create(slot->obj, NULL);
	lv_label_set_text(label, slot->widget->short_name);
	lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
	lv_obj_add_style(label, LV_LABEL_PART_MAIN, &gui.styles.widget_label);
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
