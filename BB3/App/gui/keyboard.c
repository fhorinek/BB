/*
 * keyboard.c
 *
 *  Created on: Aug 27, 2020
 *      Author: horinek
 */


#include "keyboard.h"
#include "gui_list.h"

#define COLOR_CLOSE "#ff0000 " LV_SYMBOL_CLOSE "#"
#define COLOR_APPLY "#00ff00 " LV_SYMBOL_OK "#"

static const char * default_kb_map_lc[] = {"1##", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", LV_SYMBOL_BACKSPACE, "\n",
                                                 "ABC", "a", "s", "d", "f", "g", "h", "j", "k", "l", LV_SYMBOL_NEW_LINE, "\n",
                                                 "_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":", "\n",
                                                 COLOR_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, COLOR_APPLY, ""
                                                };

static const lv_btnmatrix_ctrl_t default_kb_ctrl_lc_map[] = {
    LV_KEYBOARD_CTRL_BTN_FLAGS | 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7,
    LV_KEYBOARD_CTRL_BTN_FLAGS | 6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    LV_BTNMATRIX_CTRL_DISABLED | 2, 2, 6, 2, LV_BTNMATRIX_CTRL_DISABLED | 2
};

static const char * default_kb_map_uc[] = {"1##", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", LV_SYMBOL_BACKSPACE, "\n",
                                                 "abc", "A", "S", "D", "F", "G", "H", "J", "K", "L", LV_SYMBOL_NEW_LINE, "\n",
                                                 "_", "-", "Z", "X", "C", "V", "B", "N", "M", ".", ",", ":", "\n",
                                                 COLOR_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, COLOR_APPLY, ""
                                                };

static const lv_btnmatrix_ctrl_t default_kb_ctrl_uc_map[] = {
    LV_KEYBOARD_CTRL_BTN_FLAGS | 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7,
    LV_KEYBOARD_CTRL_BTN_FLAGS | 6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    LV_BTNMATRIX_CTRL_DISABLED | 2, 2, 6, 2, LV_BTNMATRIX_CTRL_DISABLED | 2
};

static const char * default_kb_map_spec[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", LV_SYMBOL_BACKSPACE, "\n",
                                                   "abc", "+", "-", "/", "*", "=", "%", "!", "?", "#", "<", ">", "\n",
                                                   "\\",  "@", "$", "(", ")", "{", "}", "[", "]", ";", "\"", "'", "\n",
                                                   COLOR_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, COLOR_APPLY, ""
                                                  };

static const lv_btnmatrix_ctrl_t default_kb_ctrl_spec_map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
    LV_KEYBOARD_CTRL_BTN_FLAGS | 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    LV_BTNMATRIX_CTRL_DISABLED | 2, 2, 6, 2, LV_BTNMATRIX_CTRL_DISABLED | 2
};

void keyboard_obj_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CANCEL)
	{
        if (gui.keyboard.area != NULL)
        {
            lv_event_send(gui.keyboard.area, LV_EVENT_LEAVE, NULL);
        }
        keyboard_hide();
	}
	else if (event == LV_EVENT_FOCUSED)
	{
	    if (gui.keyboard.area != NULL)
	    {
            if (!lv_group_get_editing(lv_obj_get_group(gui.keyboard.area)))
            {
                //keyboard_hide();
            }
	    }
	}
	else if (event == LV_EVENT_APPLY)
	{
	    if (gui.keyboard.area != NULL)
	    {
	        lv_event_send(gui.keyboard.area, LV_EVENT_APPLY, NULL);
	    }
	}
	else if (event == LV_EVENT_KEY)
	{
	    uint32_t key = *((uint32_t*) lv_event_get_data());
	    if (key == LV_KEY_HOME && gui.keyboard.area != NULL)
        {
            lv_event_send(gui.keyboard.area, LV_EVENT_APPLY, NULL);
        }
	}
	else if (event == LV_EVENT_VALUE_CHANGED)
	{
	    lv_keyboard_ext_t * ext = lv_obj_get_ext_attr(gui.keyboard.obj);
	    lv_keyboard_mode_t old_mode = ext->mode;

	    const char * txt = lv_btnmatrix_get_active_btn_text(gui.keyboard.obj);
	    if(txt == NULL) return;

	    if(strcmp(txt, "1##") == 0)
	    {
	        ext->mode = LV_KEYBOARD_MODE_SPECIAL;
        }
	    else if(strcmp(txt, "abc") == 0)
	    {
            ext->mode = LV_KEYBOARD_MODE_TEXT_LOWER;
        }
	    else if(strcmp(txt, "ABC") == 0)
        {
            ext->mode = LV_KEYBOARD_MODE_TEXT_UPPER;
	    }
	    else
	    {
	        lv_keyboard_def_event_cb(obj, event);
	    }

		if (old_mode != ext->mode)
		{
		    switch (ext->mode)
		    {
		        case(LV_KEYBOARD_MODE_TEXT_LOWER):
                    lv_btnmatrix_set_map(gui.keyboard.obj, default_kb_map_lc);
                    lv_btnmatrix_set_ctrl_map(gui.keyboard.obj, default_kb_ctrl_lc_map);
                break;

		        case(LV_KEYBOARD_MODE_TEXT_UPPER):
                    lv_btnmatrix_set_map(gui.keyboard.obj, default_kb_map_uc);
                    lv_btnmatrix_set_ctrl_map(gui.keyboard.obj, default_kb_ctrl_uc_map);
                break;

		        case(LV_KEYBOARD_MODE_SPECIAL):
                    lv_btnmatrix_set_map(gui.keyboard.obj, default_kb_map_spec);
                    lv_btnmatrix_set_ctrl_map(gui.keyboard.obj, default_kb_ctrl_spec_map);
		    }
		}
	}
}

void keyboard_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_FOCUSED)
	{
		if (lv_group_get_editing(gui.input.group))
		{
            keyboard_show(obj);
		}
		else
		{
            keyboard_hide();
            gui_list_event_cb(obj, event);
		}
	}
	else
	{
		gui_list_event_cb(obj, event);
	}
}

void keyboard_create()
{
	gui.keyboard.obj = lv_keyboard_create(lv_layer_sys(), NULL);
	lv_btnmatrix_set_recolor(gui.keyboard.obj, true);
	lv_obj_set_size(gui.keyboard.obj, LV_HOR_RES, GUI_KEYBOARD_SIZE);
	lv_obj_set_pos(gui.keyboard.obj, 0, LV_VER_RES);
	lv_keyboard_set_cursor_manage(gui.keyboard.obj, true);

    lv_btnmatrix_set_map(gui.keyboard.obj, default_kb_map_lc);
    lv_btnmatrix_set_ctrl_map(gui.keyboard.obj, default_kb_ctrl_lc_map);

	lv_obj_set_event_cb(gui.keyboard.obj, keyboard_obj_event_cb);

	gui.keyboard.showed = false;
}

void keyboard_anim_cb(void * obj, lv_anim_value_t val)
{
	lv_obj_set_y(gui.keyboard.obj, LV_VER_RES - val);
	lv_obj_set_height(lv_scr_act(), LV_VER_RES - val);

	if (gui.input.focus != NULL)
	{
		lv_obj_t * par = lv_obj_get_parent(gui.input.focus);
	    lv_obj_set_size(par, LV_HOR_RES, LV_VER_RES - GUI_STATUSBAR_HEIGHT - val);

	    if (gui.keyboard.area != NULL)
	    	lv_page_focus(gui.input.focus, gui.keyboard.area, LV_ANIM_OFF);
	}
}

void keyboard_show(lv_obj_t * area)
{
	if (gui.keyboard.showed)
		return;

	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_values(&a, 0, GUI_KEYBOARD_SIZE);
	lv_anim_set_exec_cb(&a, keyboard_anim_cb);
//	lv_anim_set_ready_cb(&a, "flash buttons")

	lv_group_add_obj(lv_obj_get_group(area), gui.keyboard.obj);
	lv_group_focus_obj(gui.keyboard.obj);
	lv_obj_move_foreground(gui.keyboard.obj);
	lv_group_set_editing(lv_obj_get_group(area), true);

	lv_keyboard_set_cursor_manage(gui.keyboard.obj, true);
	lv_keyboard_set_textarea(gui.keyboard.obj, area);

	gui.keyboard.area = area;
	gui.keyboard.showed = true;

	lv_anim_start(&a);
}

void keyboard_hide()
{
	if (!gui.keyboard.showed)
		return;

	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_values(&a, GUI_KEYBOARD_SIZE, 0);
	lv_anim_set_exec_cb(&a, keyboard_anim_cb);
	lv_anim_start(&a);

	lv_keyboard_set_textarea(gui.keyboard.obj, NULL);

	lv_group_remove_obj(gui.keyboard.obj);
	lv_group_set_editing(lv_obj_get_group(gui.keyboard.area), false);
	lv_group_focus_obj(gui.keyboard.area);

	gui.keyboard.area = NULL;
	gui.keyboard.showed = false;

	lv_anim_start(&a);
}
