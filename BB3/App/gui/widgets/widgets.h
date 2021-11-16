/*
 * widgets.h
 *
 *  Created on: 10. 8. 2020
 *      Author: horinek
 */

#ifndef GUI_WIDGETS_WIDGETS_H_
#define GUI_WIDGETS_WIDGETS_H_

#include "../../common.h"
#include "gui/gui.h"
#include "../../lib/lvgl/lvgl.h"
#include "widget_flags.h"

#define DECLARE_WIDGET(name)	extern widget_t widget_ ## name
#define LIST_WIDGET(name)       &widget_ ## name

#define WIDGET_ACTION_LEFT      0
#define WIDGET_ACTION_RIGHT     1
#define WIDGET_ACTION_MENU      2
#define WIDGET_ACTION_HOLD      3
#define WIDGET_ACTION_DEFOCUS   4
#define WIDGET_ACTION_CLOSE     5
#define WIDGET_ACTION_MIDDLE    6

#define WIDGET_MIN_W        24
#define WIDGET_MIN_H        16
#define WIDGET_DEFAULT_W    80
#define WIDGET_DEFAULT_H    64

#define WIDGET_ARROW_POINTS 5

typedef struct _widget_slot_t widget_slot_t;

typedef struct _widget_t
{
	//short name for widget label and cfg file
	char * short_name;
    //Long name for menu
    char * name;

	//minimum allowed size
    uint16_t w_min;
    uint16_t h_min;

	//widget init code (mandatory)
	void (* init)(lv_obj_t *, widget_slot_t *);
	//widget deinit code (mandatory)
	void (* stop)(widget_slot_t *);
	//widget update (optional, can be NULL)
	void (* update)(widget_slot_t *);
	//widget edit (optional, can be NULL)
	void (* edit)(widget_slot_t *, uint8_t);

	uint16_t vars_size;
	uint16_t flags;
} widget_t;

#define REGISTER_WIDGET_ISUE(short_name, name, w_min, h_min, flags, ...) \
    typedef struct \
    { \
        __VA_ARGS__ \
    } widget_local_vars_t; \
    static void short_name ## _init(lv_obj_t *, widget_slot_t *); \
    static void short_name ## _stop(widget_slot_t *); \
    static void short_name ## _update(widget_slot_t *); \
    static void short_name ## _edit(widget_slot_t *, uint8_t); \
    widget_t widget_ ## short_name = \
    { \
        #short_name, \
        name, \
        w_min, \
        h_min, \
        short_name ## _init, \
        short_name ## _stop, \
        short_name ## _update, \
        short_name ## _edit, \
        sizeof(widget_local_vars_t), \
		flags \
    };

#define REGISTER_WIDGET_IUE(short_name, name, w_min, h_min, flags, ...) \
    typedef struct \
    { \
        __VA_ARGS__ \
    } widget_local_vars_t; \
    static void short_name ## _init(lv_obj_t *, widget_slot_t *); \
    static void short_name ## _update(widget_slot_t *); \
    static void short_name ## _edit(widget_slot_t *, uint8_t); \
    widget_t widget_ ## short_name = \
    { \
        #short_name, \
        name, \
        w_min, \
        h_min, \
        short_name ## _init, \
        NULL, \
        short_name ## _update, \
        short_name ## _edit, \
        sizeof(widget_local_vars_t), \
		flags \
    };

#define REGISTER_WIDGET_I(short_name, name, w_min, h_min, flags, ...) \
    typedef struct \
    { \
        __VA_ARGS__ \
    } widget_local_vars_t; \
    static void short_name ## _init(lv_obj_t *, widget_slot_t *); \
    widget_t widget_ ## short_name = \
    { \
        #short_name, \
        name, \
        w_min, \
        h_min, \
        short_name ## _init, \
        NULL, \
        NULL, \
        NULL, \
        sizeof(widget_local_vars_t), \
		flags \
    };

#define REGISTER_WIDGET_IU(short_name, name, w_min, h_min, flags, ...) \
    typedef struct \
    { \
        __VA_ARGS__ \
    } widget_local_vars_t; \
    static void short_name ## _init(lv_obj_t *, widget_slot_t *); \
    static void short_name ## _update(widget_slot_t *); \
    widget_t widget_ ## short_name = \
    { \
        #short_name, \
        name, \
        w_min, \
        h_min, \
        short_name ## _init, \
        NULL, \
        short_name ## _update, \
        NULL, \
        sizeof(widget_local_vars_t), \
		flags \
    };

#define REGISTER_WIDGET_ISU(short_name, name, w_min, h_min, flags, ...) \
    typedef struct \
    { \
        __VA_ARGS__ \
    } widget_local_vars_t; \
    static void short_name ## _init(lv_obj_t *, widget_slot_t *); \
    static void short_name ## _stop(widget_slot_t *); \
    static void short_name ## _update(widget_slot_t *); \
    widget_t widget_ ## short_name = \
    { \
        #short_name, \
        name, \
        w_min, \
        h_min, \
        short_name ## _init, \
        short_name ## _stop, \
        short_name ## _update, \
        NULL, \
        sizeof(widget_local_vars_t), \
		flags \
    };

#define WIDGET_FLAGS_LEN	8

typedef struct _widget_slot_t
{
    //widget type description
	widget_t * widget;

    //gui object handle
    lv_obj_t * obj;
    lv_obj_t * title;

    //private data
    void * vars;

	//widget size and position
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;

    //flags
    char flags[WIDGET_FLAGS_LEN];
} widget_slot_t;

typedef struct
{
	lv_obj_t * base;
	widget_slot_t * widget_slots;

    uint8_t number_of_widgets;
    uint8_t _pad[3];
} page_layout_t;

//widget list
extern widget_t * widgets[];
//grid
#define WIDGET_GRID     8

uint8_t number_of_widgets();

bool widgets_load_from_file_abs(page_layout_t * page, char * path);
bool widgets_load_from_file(page_layout_t * page, char * layout_name);
bool widgets_save_to_file(page_layout_t * page, char * layout_name);
void widgets_sort_page(page_layout_t * page);

void widgets_create_base(page_layout_t * page, lv_obj_t * par);
void widgets_init_page(page_layout_t * page, lv_obj_t * par);
void widgets_deinit_page(page_layout_t * page);
void widgets_free(page_layout_t * page);

lv_obj_t * widgets_add_page_empty_label(page_layout_t * page);

widget_t * widget_find_by_name(char * name);
void widget_init(widget_slot_t * ws, lv_obj_t * par);
void widget_update(widget_slot_t * ws);
void widget_deinit(widget_slot_t * ws);

bool widgets_editable(page_layout_t * page);
void widgets_edit(widget_slot_t * ws, uint8_t action);
widget_slot_t * widgets_editable_select_next(page_layout_t * page, widget_slot_t * last);

bool widget_flag_is_set(widget_slot_t * slot, widget_flags_id_t flag_id);
void widget_flag_set(widget_slot_t * slot, widget_flags_id_t flag_id);
void widget_flag_unset(widget_slot_t * slot, widget_flags_id_t flag_id);

void widgets_add(page_layout_t * page, widget_t * w);
void widgets_remove(page_layout_t * page, uint8_t index);
void widgets_change(page_layout_t * page, uint8_t index, widget_t * w);
void widgets_update(page_layout_t * page);

#endif /* GUI_WIDGETS_WIDGETS_H_ */
