/*
 * gui.h
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */

#ifndef GUI_GUI_H_
#define GUI_GUI_H_

#include "../common.h"
#include "../lib/lvgl/lvgl.h"

typedef void (* gui_list_task_cb_t)(lv_obj_t *, lv_event_t, uint8_t);

#define NUMBER_OF_WIDGET_FONTS	5

typedef struct
{
	lv_obj_t * (* init)(lv_obj_t *);
	void (* loop)();
	void (* stop)();
	void ** local_vars;
	uint16_t local_vars_size;
} gui_task_t;

typedef struct
{
	//active task
	struct
	{
		gui_task_t * last;
		gui_task_t * actual;
		lv_scr_load_anim_t next_anim;
	} task;

	//input group
	struct
	{
		lv_group_t * nav;
		lv_group_t * keypad;
		lv_obj_t * focus;
	} input;

	//list
	struct
	{
		lv_obj_t * object;
		gui_list_task_cb_t callback;
	} list;

	//statusbar
	struct
	{
		lv_obj_t * bar;
		lv_obj_t * time;
		lv_obj_t * icons;

		lv_obj_t * mbox;
	} statusbar;

	//display driver
	lv_disp_drv_t disp_drv;

	struct
	{
		lv_obj_t * obj;
		lv_obj_t * area;
		bool showed;
	} keyboard;


	//styles
	struct
	{
		lv_style_t widget_label;
		lv_style_t widget_box;
		const lv_font_t * widget_fonts[NUMBER_OF_WIDGET_FONTS];
	} styles;


} gui_t;

void gui_set_group_focus(lv_obj_t * obj);
void gui_switch_task(gui_task_t * next, lv_scr_load_anim_t anim);

void gui_init();
void gui_loop();
void gui_stop();

#define GUI_TASK_SW_ANIMATION	250
#define GUI_STATUSBAR_HEIGHT	24

#define GUI_INDICATOR_HEIGHT	30
#define GUI_INDICATOR_Y_POS		280
#define GUI_INDICATOR_DOT_SIZE	16
#define GUI_INDICATOR_DELAY		2000
#define GUI_INDICATOR_ANIM		200

#define GUI_MSG_TIMEOUT			5000

#define GUI_KEYBOARD_SIZE		(LV_VER_RES / 3)


extern gui_t gui;

#define DECLARE_TASK(name) \
	extern gui_task_t gui_ ## name;

#define REGISTER_TASK_ILS(name, ...) \
	typedef struct \
	{ \
		__VA_ARGS__ \
	} gui_local_vars_t; \
	static gui_local_vars_t * local = NULL;\
	static lv_obj_t * name ## _init(lv_obj_t * par); \
	static void name ## _loop(); \
	static void name ## _stop(); \
	gui_task_t gui_ ## name = \
	{ \
		name ## _init, \
		name ## _loop, \
		name ## _stop, \
		(void *)&local, \
		sizeof(gui_local_vars_t)\
	};\

#define REGISTER_TASK_IS(name, ...) \
	typedef struct \
	{ \
		__VA_ARGS__ \
	} gui_local_vars_t; \
	static gui_local_vars_t * local = NULL;\
	static lv_obj_t * name ## _init(lv_obj_t * par); \
	static void name ## _stop(); \
	gui_task_t gui_ ## name = \
	{ \
		name ## _init, \
		NULL, \
		name ## _stop, \
		(void *)&local, \
		sizeof(gui_local_vars_t)\
	};\

#define REGISTER_TASK_I(name, ...) \
	typedef struct \
	{ \
		__VA_ARGS__ \
	} gui_local_vars_t; \
	static gui_local_vars_t * local = NULL;\
	static lv_obj_t * name ## _init(lv_obj_t * par); \
	gui_task_t gui_ ## name = \
	{ \
		name ## _init, \
		NULL, \
		NULL, \
		(void *)&local, \
		sizeof(gui_local_vars_t)\
	};\

#define REGISTER_TASK_IL(name, ...) \
	typedef struct \
	{ \
		__VA_ARGS__ \
	} gui_local_vars_t; \
	static gui_local_vars_t * local = NULL;\
	static lv_obj_t * name ## _init(lv_obj_t * par); \
	static void name ## _loop(); \
	gui_task_t gui_ ## name = \
	{ \
		name ## _init, \
		name ## _loop, \
		NULL, \
		(void *)&local, \
		sizeof(gui_local_vars_t)\
	};\

#endif /* GUI_GUI_H_ */
