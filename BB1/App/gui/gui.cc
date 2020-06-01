/*
 * gui.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "gui.h"

#include "gui_list.h"

#include "settings.h"
#include "fanet.h"
#include "pages.h"

#include "../lib/lvgl/src/lv_misc/lv_gc.h"

#define lv_last_anim ((lv_anim_t *)(_lv_ll_get_tail(&LV_GC_ROOT(_lv_anim_ll))))

gui_task_t * gui_task_actual;
gui_task_t * gui_task_next;
gui_switch_anim_t gui_task_next_anim;

//input group
lv_group_t * gui_group;

//active page
lv_obj_t * gui_page;
lv_obj_t * gui_page_focus;
lv_obj_t * gui_page_old;

//base tileview
lv_obj_t * gui_tileview;

void gui_set_page_focus(lv_obj_t * obj)
{
	gui_page_focus = obj;
}

void gui_switch_task(gui_task_t * next, gui_switch_anim_t anim)
{
	gui_task_next = next;
	gui_task_next_anim = anim;
}

void gui_destroy_page(lv_obj_t * obj)
{
	lv_obj_del(obj);
	gui_page_old = NULL;
}

static void gui_group_focus_cb(lv_group_t * group)
{
	if (!gui_page_focus)
		return;

    lv_obj_t * focused = lv_group_get_focused(group);
    if (gui_page != NULL)
    	lv_page_focus(gui_page_focus, focused, LV_ANIM_ON);
}

void gui_switch_cb(lv_anim_t * a)
{
	(void)a;

	//end animation

	//set new tile to 0,0
	lv_obj_set_pos(gui_page, 0, 0);
	lv_tileview_set_tile_act(gui_tileview, 0, 0, LV_ANIM_OFF);

	//destrou last tile
	gui_destroy_page(gui_page_old);
}


void gui_init()
{
	gui_list_init();

	//Create base layer
    gui_tileview = lv_tileview_create(lv_scr_act(), NULL);
    lv_tileview_set_edge_flash(gui_tileview, true);
    //lv_page_set_sb_mode(gui_tileview, LV_SB_MODE_OFF);

    static lv_point_t valid_pos[] = {{0,0}, {0, 1}, {1, 0}};
    lv_tileview_set_valid_positions(gui_tileview, valid_pos, 3);

	//first screen
	gui_task_next = NULL;
	gui_task_actual = &gui_fanet;
	gui_page = gui_task_actual->init(gui_tileview);

	gui_page_old = NULL;
	gui_page_focus = NULL;

	//set group focus callback
	lv_group_set_focus_cb(gui_group, gui_group_focus_cb);
}


void gui_loop()
{
	//execute task
	gui_task_actual->loop();

	if (gui_task_next != NULL)
	{
		//stop actual task
		if (!gui_task_actual->stop())
		{
			//task refuse to stop
			gui_task_next = NULL;
			return;
		}

		gui_page_old = gui_page;
		gui_page = NULL;
		gui_page_focus = NULL;

		//switch taks
		gui_task_actual = gui_task_next;

		//init new task
		lv_group_remove_all_objs(gui_group);
		gui_page = gui_task_actual->init(gui_tileview);

		switch (gui_task_next_anim)
		{
			case(GUI_SW_NONE):
				lv_obj_set_pos(gui_page, 0, 0);
				gui_destroy_page(gui_page_old);
				break;

			case(GUI_SW_TOP_BOTTOM):
				lv_obj_set_pos(gui_page, 0, 0);
				lv_obj_set_pos(gui_page_old, 0, LV_VER_RES);
				lv_tileview_set_tile_act(gui_tileview, 0, 1, LV_ANIM_OFF);
				lv_tileview_set_tile_act(gui_tileview, 0, 0, LV_ANIM_ON);
				lv_last_anim->ready_cb = gui_switch_cb;
				break;

			case(GUI_SW_BOTTOM_TOP):
				lv_obj_set_pos(gui_page, 0, LV_VER_RES);
				lv_obj_set_pos(gui_page_old, 0, 0);
				lv_tileview_set_tile_act(gui_tileview, 0, 0, LV_ANIM_OFF);
				lv_tileview_set_tile_act(gui_tileview, 0, 1, LV_ANIM_ON);
				lv_last_anim->ready_cb = gui_switch_cb;
				break;

			case(GUI_SW_LEFT_RIGHT):
				lv_obj_set_pos(gui_page, 0, 0);
				lv_obj_set_pos(gui_page_old, LV_HOR_RES, 0);
				lv_tileview_set_tile_act(gui_tileview, 1, 0, LV_ANIM_OFF);
				lv_tileview_set_tile_act(gui_tileview, 0, 0, LV_ANIM_ON);
				lv_last_anim->ready_cb = gui_switch_cb;
				break;

			case(GUI_SW_RIGHT_LEFT):
				lv_obj_set_pos(gui_page, LV_HOR_RES, 0);
				lv_obj_set_pos(gui_page_old, 0, 0);
				lv_tileview_set_tile_act(gui_tileview, 0, 0, LV_ANIM_OFF);
				lv_tileview_set_tile_act(gui_tileview, 1, 0, LV_ANIM_ON);
				lv_last_anim->ready_cb = gui_switch_cb;
				break;
		}

		gui_task_next = NULL;
	}
}
