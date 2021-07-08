/*
 * gui.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "gui.h"

#include "gui_list.h"
#include "statusbar.h"
#include "keyboard.h"

#include "tasks/page/pages.h"
#include "lib/lvgl/src/lv_misc/lv_gc.h"

#include "drivers/tft/tft.h"

gui_t gui;

void gui_set_loop_speed(uint16_t speed)
{
    gui.task.loop_speed = speed;
}

void gui_set_dummy_event_cb(lv_obj_t * par, lv_event_cb_t event_cb)
{
    lv_obj_t * dummy = lv_obj_create(par, NULL);
    lv_obj_set_size(dummy, 0, 0);
    lv_group_add_obj(gui.input.group, dummy);
    lv_group_set_editing(gui.input.group, true);
    lv_obj_set_event_cb(dummy, event_cb);
}


void gui_set_group_focus(lv_obj_t * obj)
{
	gui.input.focus = obj;
}

static void gui_group_focus_cb(lv_group_t * group)
{
	if (!gui.input.focus)
		return;

    lv_obj_t * focused = lv_group_get_focused(group);

    if (gui.input.focus != NULL)
    	lv_page_focus(gui.input.focus, focused, LV_ANIM_ON);
}

//when task screen is deleted, trigger task stop, clear memory
void screen_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_DELETE)
	{
		if (gui.task.last != NULL)
		{
			if (gui.task.last->stop != NULL)
			{
				gui.task.last->stop();
			}

			if (*gui.task.last->local_vars != NULL)
			{
				free(*gui.task.last->local_vars);
				*gui.task.last->local_vars = NULL;
			}
		}
	}
}

lv_obj_t * gui_task_create(gui_task_t * task)
{
	lv_obj_t * screen = lv_obj_create(NULL, NULL);
	lv_obj_t * base = lv_obj_create(screen, NULL);

    lv_obj_set_pos(base, 0, GUI_STATUSBAR_HEIGHT);
    lv_obj_set_size(base, LV_HOR_RES, LV_VER_RES - GUI_STATUSBAR_HEIGHT);

    //allocate memory for local task
    if (task->local_vars_size > 0)
    {
    	*task->local_vars = malloc(task->local_vars_size);
    	ASSERT(*task->local_vars != NULL);
    }
    else
    {
    	*task->local_vars = NULL;
    }

	//init
	task->init(base);
	if (task->loop != NULL)
	    task->loop();

	lv_obj_set_event_cb(screen, screen_event_cb);

	return screen;
}


void gui_switch_task(gui_task_t * next, lv_scr_load_anim_t anim)
{
	gui.input.focus = NULL;

    //reset input dev
    lv_indev_reset(gui.input.indev, NULL);
    lv_indev_wait_release(gui.input.indev);

	//remove input groups from old screen
	lv_group_remove_all_objs(gui.input.group);
	lv_group_set_editing(gui.input.group, false);

	//switch task
	gui.task.last = gui.task.actual;
	gui.task.actual = next;

	gui_set_loop_speed(GUI_DEFAULT_LOOP_SPEED);

	//init new screen
	lv_obj_t * screen = gui_task_create(gui.task.actual);

	//switch screens
	lv_scr_load_anim(screen, anim, GUI_TASK_SW_ANIMATION, 0, true);
}


void gui_init_styles()
{
    lv_style_init(&gui.styles.widget_label);
    lv_style_set_text_font(&gui.styles.widget_label, LV_STATE_DEFAULT, &lv_font_montserrat_12);

    lv_style_init(&gui.styles.widget_unit);
    lv_style_set_text_font(&gui.styles.widget_unit, LV_STATE_DEFAULT, &lv_font_montserrat_12);

	lv_style_init(&gui.styles.widget_box);
	lv_style_set_pad_all(&gui.styles.widget_label, LV_STATE_DEFAULT, 2);

	lv_style_init(&gui.styles.list_select);
	lv_style_set_radius(&gui.styles.list_select, LV_STATE_FOCUSED, 5);
	lv_style_set_radius(&gui.styles.list_select, LV_STATE_EDITED, 5);

	gui.styles.widget_fonts[0] = &lv_font_montserrat_44;
	gui.styles.widget_fonts[1] = &lv_font_montserrat_34;
	gui.styles.widget_fonts[2] = &lv_font_montserrat_28;
	gui.styles.widget_fonts[3] = &lv_font_montserrat_16;
	gui.styles.widget_fonts[4] = &lv_font_montserrat_12;
}

void gui_stop()
{
	tft_wait_for_buffer();
	tft_color_fill(0xFFFF);
	tft_refresh_buffer(0, 0, 239, 399);
	tft_wait_for_buffer();
	tft_stop();
}

void gui_init()
{
    gui.take_screenshot = false;
    gui.dialog.active = false;

	gui_init_styles();

	//create statusbar
	statusbar_create();

	//create keyboard
	keyboard_create();

	//set group focus callback
	gui.input.focus = NULL;
	lv_group_set_focus_cb(gui.input.group, gui_group_focus_cb);

	//first task
	gui.task.last = NULL;
	gui.task.actual = &gui_pages;

	//list
	gui.list.back = NULL;
	gui.list.entry_list = NULL;

	//load the screen
	lv_obj_t * screen = gui_task_create(gui.task.actual);

	if (gui.task.actual == &gui_pages)
		pages_splash_show();

	lv_scr_load(screen);
}

void gui_loop()
{
	static uint32_t next_update = 0;

	if (next_update < HAL_GetTick())
	{
		next_update = HAL_GetTick() + gui.task.loop_speed;

		statusbar_step();

		//execute task
		if (gui.task.actual->loop != NULL)
		{
			gui.task.actual->loop();
		}
	}
}
