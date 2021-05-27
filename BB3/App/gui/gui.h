/*
 * gui.h
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */

#ifndef GUI_GUI_H_
#define GUI_GUI_H_

#include "common.h"
#include "lib/lvgl/lvgl.h"

typedef void (* gui_list_task_cb_t)(lv_obj_t *, lv_event_t, uint8_t);
typedef void (* gui_dialog_cb_t)(uint8_t, void * data);

typedef enum
{
    dialog_yes_no,
    dialog_progress,
    dialog_textarea,
    dialog_confirm,
} dialog_type_t;

typedef enum
{
    dialog_res_none,
    dialog_res_yes,
    dialog_res_no,
    dialog_res_cancel,
} dialog_result_t;


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

		uint16_t loop_speed;
		lv_scr_load_anim_t next_anim;
		uint8_t _pad[1];
	} task;

	//input group
	struct
	{
        lv_indev_t * indev;
		lv_group_t * group;
		lv_obj_t * focus;
	} input;

	//list
	struct
	{
		lv_obj_t * object;
		gui_list_task_cb_t callback;
	} list;

	//dialog
	struct
	{
        lv_obj_t * window;
        lv_group_t * group;

        uint8_t type;
        gui_dialog_cb_t cb;
        bool active;
        uint8_t _pad[1];
	} dialog;

	//statusbar
	struct
	{
		lv_obj_t * bar;
		lv_obj_t * time;
        lv_obj_t * icons;
        lv_obj_t * gray_icons;

		lv_obj_t * mbox;
	} statusbar;


	struct
	{
		lv_obj_t * obj;
		lv_obj_t * area;

		bool showed;
		uint8_t _pad[3];
	} keyboard;

	//styles
	struct
	{
        lv_style_t widget_label;
        lv_style_t widget_unit;
		lv_style_t widget_box;
		lv_style_t list_select;
		const lv_font_t * widget_fonts[NUMBER_OF_WIDGET_FONTS];
	} styles;


	struct
	{
        lv_obj_t * canvas;
        int32_t map_lat;
        int32_t map_lan;

        lv_color_t * buffer[2];

        uint8_t active_buffer;
        uint8_t magic;
        uint8_t _pad[2];
	} map;

	osSemaphoreId_t lock;

	uint8_t take_screenshot;
    uint8_t _pad[3];

} gui_t;

void gui_set_group_focus(lv_obj_t * obj);
void gui_switch_task(gui_task_t * next, lv_scr_load_anim_t anim);

void gui_set_loop_speed(uint16_t speed);
void gui_set_dummy_event_cb(lv_obj_t * par, lv_event_cb_t event_cb);

void gui_init();
void gui_loop();
void gui_stop();

void gui_take_screenshot();
void gui_lock_acquire();
void gui_lock_release();

#define GUI_TASK_SW_ANIMATION	250
#define GUI_STATUSBAR_HEIGHT	24

#define GUI_INDICATOR_HEIGHT	30
#define GUI_INDICATOR_Y_POS		280
#define GUI_INDICATOR_DOT_SIZE	16
#define GUI_INDICATOR_DELAY		2000
#define GUI_INDICATOR_ANIM		200

#define GUI_MSG_TIMEOUT			5000

#define GUI_KEYBOARD_SIZE		(LV_VER_RES / 3)

#define GUI_DEFAULT_LOOP_SPEED  200

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
