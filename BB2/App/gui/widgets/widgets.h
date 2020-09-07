/*
 * widgets.h
 *
 *  Created on: 10. 8. 2020
 *      Author: horinek
 */

#ifndef GUI_WIDGETS_WIDGETS_H_
#define GUI_WIDGETS_WIDGETS_H_

#include "../../common.h"
#include "../gui.h"
#include "../../lib/lvgl/lvgl.h"

#define DECLARE_WIDGET(name)	extern widget_t widget_ ## name

typedef struct _widget_slot_t widget_slot_t;

typedef struct _widget_t
{
	//Long name for menu
	char * name;
	//short name for widget label and cfg file
	char * short_name;

	//widget init code (mandatory)
	void (* init)(lv_obj_t *, widget_slot_t *, uint16_t, uint16_t, uint16_t, uint16_t);
	//widget deinit code (mandatory)
	void (* stop)(widget_slot_t *);
	//widget update (optional, can be NULL)
	void (* update)(widget_slot_t *);
	//widget irqh (optional, can be NULL)
	void (* irqh)(widget_slot_t *, uint8_t);

	uint16_t vars_size;
} widget_t;

typedef struct _widget_slot_t
{
	widget_t * widget;
	lv_obj_t * obj;
	void * vars;

} widget_slot_t;

typedef struct
{
	uint8_t number_of_widgets;
	lv_obj_t * base;
	widget_slot_t ** widgets;
} page_layout_t;

//widget list
extern widget_t * widgets[];

uint8_t number_of_widgets();

bool widgets_load_from_file(page_layout_t * page, char * name);
void widgets_unload(page_layout_t * page);

bool widgets_editable(page_layout_t * page);


#endif /* GUI_WIDGETS_WIDGETS_H_ */
