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

typedef struct
{
	lv_obj_t * (* init)(lv_obj_t *);
	void (* loop)();
	bool (* stop)();
} gui_task_t;

typedef enum
{
	GUI_SW_NONE = 0,
	GUI_SW_TOP_BOTTOM,
	GUI_SW_BOTTOM_TOP,
	GUI_SW_LEFT_RIGHT,
	GUI_SW_RIGHT_LEFT
} gui_switch_anim_t;

extern lv_group_t * gui_group;

void gui_set_backlight(uint8_t val);

void gui_set_page_focus(lv_obj_t * obj);
void gui_switch_task(gui_task_t * next, gui_switch_anim_t anim);

void gui_init();
void gui_loop();

#define GUI_PAGE_MEMORY_SIZE	128
extern uint8_t gui_page_memory[GUI_PAGE_MEMORY_SIZE];

#define local	(*((local_vars_t *)(&gui_page_memory)))

#endif /* GUI_GUI_H_ */
