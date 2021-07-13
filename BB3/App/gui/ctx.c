/*
 * ctx.c
 *
 *  Created on: Jul 13, 2021
 *      Author: horinek
 */
#include "ctx.h"

void ctx_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        uint8_t option = lv_dropdown_get_selected(gui.ctx.dropdown);

        lv_obj_t * last_focus = gui.ctx.last_focus;
    	ctx_close();
        gui.ctx.cb(option, last_focus);
    }

    if (event == LV_EVENT_CANCEL || event == LV_EVENT_CLICKED)
    {
        lv_obj_t * last_focus = gui.ctx.last_focus;
    	ctx_close();
    	gui.ctx.cb(CTX_CANCEL, last_focus);
    }
}

void ctx_close()
{
	ctx_show();

	//refocus old object
	lv_group_remove_obj(gui.ctx.dropdown);
	if (gui.ctx.last_focus != NULL)
	{
		lv_group_focus_obj(gui.ctx.last_focus);
		gui.ctx.last_focus = NULL;
	}

}

void ctx_open()
{
	ctx_hide();
	lv_dropdown_set_selected(gui.ctx.dropdown, 0);

	//save last
	gui.ctx.last_focus = lv_group_get_focused(gui.input.group);

    //temoprary add to group
    lv_group_add_obj(gui.input.group, gui.ctx.dropdown);
    lv_group_focus_obj(gui.ctx.dropdown);
    lv_group_set_editing(gui.input.group, true);
}


void ctx_init()
{
	gui.ctx.last_focus = NULL;

	//init style, hint and dropdown
    //Label style
    lv_style_init(&gui.styles.ctx_menu);
    lv_style_set_bg_opa(&gui.styles.ctx_menu, LV_STATE_DEFAULT, LV_OPA_50);
    lv_style_set_pad_all(&gui.styles.ctx_menu, LV_STATE_DEFAULT, 5);
    lv_style_set_pad_bottom(&gui.styles.ctx_menu, LV_STATE_DEFAULT, 5);
    lv_style_set_radius(&gui.styles.ctx_menu, LV_STATE_DEFAULT, 5);
    lv_style_set_text_color(&gui.styles.ctx_menu, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    //hint label
    gui.ctx.hint = lv_label_create(lv_layer_sys(), NULL);
    lv_obj_add_style(gui.ctx.hint, LV_LABEL_PART_MAIN, &gui.styles.ctx_menu);
    lv_label_set_text(gui.ctx.hint, LV_SYMBOL_LIST);
    lv_label_set_long_mode(gui.ctx.hint, LV_LABEL_LONG_CROP);
    lv_label_set_align(gui.ctx.hint, LV_LABEL_ALIGN_LEFT);
    lv_obj_set_size(gui.ctx.hint, 30, 30);
    lv_obj_align(gui.ctx.hint, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 5, 5);

    lv_obj_move_foreground(gui.ctx.hint);

    //ctx menu
    gui.ctx.dropdown = lv_dropdown_create(lv_layer_sys(), NULL);
    lv_obj_set_event_cb(gui.ctx.dropdown, ctx_cb);
    lv_obj_set_size(gui.ctx.dropdown, 0, 0);
    lv_obj_align(gui.ctx.dropdown, lv_layer_sys(), LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 30);
    lv_dropdown_set_dir(gui.ctx.dropdown, LV_DROPDOWN_DIR_LEFT);

    lv_dropdown_set_show_selected(gui.ctx.dropdown, false);
    lv_dropdown_set_text(gui.ctx.dropdown, "");
    lv_dropdown_set_symbol(gui.ctx.dropdown, NULL);
    lv_dropdown_clear_options(gui.ctx.dropdown);

    ctx_hide();
}

void ctx_clear()
{
	lv_dropdown_clear_options(gui.ctx.dropdown);
}

void ctx_set_cb(gui_ctx_cb_t cb)
{
	gui.ctx.cb = cb;
}

void ctx_add_option(char * option, uint8_t pos)
{
	lv_dropdown_add_option(gui.ctx.dropdown, option, pos);
}

void ctx_show()
{
	lv_obj_set_hidden(gui.ctx.hint, false);
}

void ctx_hide()
{
	lv_obj_set_hidden(gui.ctx.hint, true);
}

