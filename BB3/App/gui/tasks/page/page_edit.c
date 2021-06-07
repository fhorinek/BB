/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include <gui/tasks/menu/settings.h>
#include "page_edit.h"

#include "page_settings.h"
#include "widget_list.h"

#include "gui/widgets/widgets.h"
#include "gui/dialog.h"
#include "gui/anim.h"

typedef enum {
    mode_select,
    mode_move_x,
    mode_move_y,
    mode_size_x,
    mode_size_y,
    _mode_cnt
} page_edit_mode_t;

REGISTER_TASK_ILS(page_edit,
	//pages
	page_layout_t page;

    page_edit_mode_t mode;
    lv_obj_t * mode_label;
    lv_obj_t * hint_label;
    lv_obj_t * dummy;

    lv_obj_t * ctx_menu;
    uint8_t ctx_focus_index;

    lv_obj_t * par;
    bool changed;

    lv_style_t label_style;

	char page_name[PAGE_NAME_LEN + 1];
	uint8_t page_index;
);

#define PAGE_EDIT_ADD_NEW   0xFF

static void page_edit_event_cb(lv_obj_t * obj, lv_event_t event);
void page_edit_open_ctx_menu(uint8_t index);
void page_edit_add_widgets_to_group();


void page_edit_set_mode(page_edit_mode_t mode)
{
    local->mode = mode;

    if (local->page.number_of_widgets == 0)
    {
        lv_group_set_editing(gui.input.group, false);
        lv_label_set_text(local->mode_label, LV_SYMBOL_PLUS " Add widget");
        lv_obj_set_hidden(local->hint_label, true);
        return;
    }

    //show hint
    lv_obj_set_hidden(local->hint_label, false);

    //set editing
    lv_group_set_editing(gui.input.group, (mode != mode_select));

    switch (mode)
    {
        case(mode_select):
            lv_label_set_text(local->mode_label, LV_SYMBOL_LEFT " select " LV_SYMBOL_RIGHT);
        break;
        case(mode_move_x):
            lv_label_set_text(local->mode_label, LV_SYMBOL_LEFT " move " LV_SYMBOL_RIGHT);
        break;
        case(mode_move_y):
            lv_label_set_text(local->mode_label, LV_SYMBOL_UP " move " LV_SYMBOL_DOWN);
        break;
        case(mode_size_x):
            lv_label_set_text(local->mode_label, LV_SYMBOL_LEFT " scale " LV_SYMBOL_RIGHT);
        break;
        case(mode_size_y):
            lv_label_set_text(local->mode_label, LV_SYMBOL_UP " scale " LV_SYMBOL_DOWN);
        break;
    }
}

void page_edit_move_widget(widget_slot_t * ws, int8_t dir)
{
    int16_t inc = WIDGET_GRID * dir;

    int16_t w = lv_obj_get_width(local->par);
    int16_t h = lv_obj_get_height(local->par);

    local->changed = true;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, ws->obj);

    uint16_t old_x = ws->x;
    uint16_t old_y = ws->y;

    switch(local->mode)
    {
        case(mode_move_x):
            ws->x = max(ws->x + inc, 0);
            ws->x = min(ws->x, w - ws->w);

            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_x);
            lv_anim_set_values(&a, old_x, ws->x);
        break;
        case(mode_move_y):
            ws->y = max(ws->y + inc, 0);
            ws->y = min(ws->y, h - ws->h);

            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);
            lv_anim_set_values(&a, old_y, ws->y);
        break;
        case(mode_size_x):
            ws->w = max(ws->w + inc, ws->widget->w_min);
            ws->w = min(ws->w, w - ws->x);
        break;
        case(mode_size_y):
            ws->h = max(ws->h + inc, ws->widget->h_min);
            ws->h = min(ws->h, h - ws->y);
        break;
    }

    if (local->mode == mode_size_x || local->mode == mode_size_y)
    {
        //to resize the widget it need to be re initilised
        widget_deinit(ws);
        lv_obj_del(ws->obj);

        widget_init(ws, local->page.base);

        lv_group_add_obj(gui.input.group, ws->obj);
        lv_obj_set_event_cb(ws->obj, page_edit_event_cb);
        lv_group_focus_obj(ws->obj);
        lv_group_set_editing(gui.input.group, true);
    }
    else
    {
        lv_anim_start(&a);
    }
}

void page_edit_remove_cb(dialog_result_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        anim_fade_out_delete(local->page.widget_slots[local->ctx_focus_index].obj);
        widgets_remove(&local->page, local->ctx_focus_index);

        local->changed = true;

        //if there are no widgets -> show message
        if (local->page.number_of_widgets == 0)
        {
            widgets_add_page_empty_label(&local->page);
            page_edit_add_widgets_to_group();
        }
    }
}

void page_edit_go_to_widget_list(uint8_t widget_index)
{
    if (local->changed)
        widgets_save_to_file(&local->page, local->page_name);

    //switch task
    gui_switch_task(&gui_widget_list, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    widget_list_set_page_name(local->page_name, local->page_index);

    if (widget_index != PAGE_EDIT_ADD_NEW)
        widget_list_select_widget(local->page.widget_slots[widget_index].widget, widget_index);
    else
        widget_list_select_widget(NULL, PAGE_EDIT_ADD_NEW);

}

static void page_edit_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_DELETE)
        return;

    uint8_t index;
    bool widget_selected = false;

    for (index = 0; index < local->page.number_of_widgets; index++)
    {
        if (local->page.widget_slots[index].obj == obj)
        {
            widget_selected = true;
            break;
        }
    }

    switch(event)
    {
        case (LV_EVENT_CANCEL):
            if (local->mode == mode_select)
            {
                if (local->changed)
                    widgets_save_to_file(&local->page, local->page_name);

                gui_switch_task(&gui_page_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
                page_settings_set_page_name(local->page_name, local->page_index);
            }
            else
            {
                page_edit_set_mode(mode_select);
            }
        break;

        case LV_EVENT_FOCUSED:
            if (widget_selected)
                lv_obj_move_foreground(obj);
        break;

        case LV_EVENT_CLICKED:
            if (local->page.number_of_widgets == 0)
            {
                page_edit_go_to_widget_list(PAGE_EDIT_ADD_NEW);
            }
            else
            {
                page_edit_set_mode((local->mode + 1) % _mode_cnt);
            }
		break;

        case LV_EVENT_KEY:
        {
        	uint32_t key = *((uint32_t *) lv_event_get_data());
        	if (widget_selected)
        	{
                switch (key)
                {
                    case(LV_KEY_RIGHT):
                        page_edit_move_widget(&local->page.widget_slots[index], +1);
                    break;

                    case(LV_KEY_LEFT):
                        page_edit_move_widget(&local->page.widget_slots[index], -1);
                    break;

                    case(LV_KEY_HOME):
                        page_edit_open_ctx_menu(index);
                    break;
                }
        	}

        }
		break;
    }

}

void page_edit_ctx_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        uint8_t option = lv_dropdown_get_selected(local->ctx_menu);

        //add
        if (option == 0)
            page_edit_go_to_widget_list(PAGE_EDIT_ADD_NEW);

        //change
        if (option == 1)
            page_edit_go_to_widget_list(local->ctx_focus_index);

        //remove
        if (option == 2)
        {
            char text[128];
            snprintf(text, sizeof(text), "Do you really want to remove widget '%s'?", local->page.widget_slots[local->ctx_focus_index].widget->name);
            dialog_show("Remove widget", text, dialog_yes_no, page_edit_remove_cb);
        }
    }

    if (event == LV_EVENT_CANCEL || event == LV_EVENT_CLICKED)
    {
        //refocus old widget
        lv_group_remove_obj(local->ctx_menu);
        page_edit_set_mode(mode_select);
        lv_group_focus_obj(local->page.widget_slots[local->ctx_focus_index].obj);
    }
}

void page_edit_open_ctx_menu(uint8_t index)
{
    local->ctx_focus_index = index;

    //temoprary add to group
    lv_group_add_obj(gui.input.group, local->ctx_menu);
    lv_group_focus_obj(local->ctx_menu);
    lv_group_set_editing(gui.input.group, true);
}

void page_edit_add_widgets_to_group()
{
    lv_group_remove_all_objs(gui.input.group);

    for (uint8_t i = 0; i < local->page.number_of_widgets; i++)
    {
        widget_slot_t * ws = &local->page.widget_slots[i];

        lv_group_add_obj(gui.input.group, ws->obj);
        lv_obj_set_event_cb(ws->obj, page_edit_event_cb);
    }

    if (local->page.number_of_widgets > 0)
    {
        lv_obj_move_foreground(lv_group_get_focused(gui.input.group));
    }
    else
    {
        lv_group_add_obj(gui.input.group, local->dummy);
    }

    page_edit_set_mode(mode_select);
}

void page_edit_set_page_name(char * filename, uint8_t index)
{
    strncpy(local->page_name, filename, sizeof(local->page_name));
    widgets_load_from_file(&local->page, filename);
    widgets_init_page(&local->page, local->par);

    local->page_index = index;

    page_edit_add_widgets_to_group();
}

void page_edit_modify_widget(widget_t * w, uint8_t widget_index)
{
    local->changed = true;
    lv_obj_t * focus_this = NULL;

    if (widget_index == PAGE_EDIT_ADD_NEW)
    {
        if (local->page.number_of_widgets == 0)
        {
            //remove empty label
            lv_obj_del(lv_obj_get_child(local->page.base, NULL));
        }

        widgets_add(&local->page, w);
        focus_this = local->page.widget_slots[local->page.number_of_widgets - 1].obj;
    }
    else
    {
        lv_obj_del(local->page.widget_slots[widget_index].obj);
        widgets_change(&local->page, widget_index, w);
        focus_this = local->page.widget_slots[widget_index].obj;
    }

    page_edit_add_widgets_to_group();
    lv_group_focus_obj(focus_this);
}


static lv_obj_t * page_edit_init(lv_obj_t * par)
{
    local->par = par;
    local->changed = false;

    //Label style
    lv_style_init(&local->label_style);
    lv_style_set_bg_opa(&local->label_style, LV_STATE_DEFAULT, LV_OPA_50);
    lv_style_set_pad_all(&local->label_style, LV_STATE_DEFAULT, 5);
    lv_style_set_pad_bottom(&local->label_style, LV_STATE_DEFAULT, 5);
    lv_style_set_radius(&local->label_style, LV_STATE_DEFAULT, 5);
    lv_style_set_text_color(&local->label_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    //mode label
    local->mode_label = lv_label_create(par, NULL);
    lv_obj_add_style(local->mode_label, LV_LABEL_PART_MAIN, &local->label_style);
    lv_label_set_long_mode(local->mode_label, LV_LABEL_LONG_CROP);
    lv_label_set_align(local->mode_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_size(local->mode_label, 120, 30);
    lv_obj_align(local->mode_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 5);
    lv_obj_set_auto_realign(local->mode_label, true);

    //hint label
    local->hint_label = lv_label_create(par, NULL);
    lv_obj_add_style(local->hint_label, LV_LABEL_PART_MAIN, &local->label_style);
    lv_label_set_text(local->hint_label, LV_SYMBOL_LIST);
    lv_label_set_long_mode(local->hint_label, LV_LABEL_LONG_CROP);
    lv_label_set_align(local->hint_label, LV_LABEL_ALIGN_LEFT);
    lv_obj_set_size(local->hint_label, 30, 30);
    lv_obj_align(local->hint_label, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 5, 5);

    lv_obj_move_foreground(local->mode_label);
    lv_obj_move_foreground(local->hint_label);

    //ctx menu
    local->ctx_menu = lv_dropdown_create(par, NULL);
    lv_obj_set_event_cb(local->ctx_menu, page_edit_ctx_cb);
    lv_obj_set_size(local->ctx_menu, 0, 0);
    lv_obj_align(local->ctx_menu, par, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 30);
    lv_dropdown_set_dir(local->ctx_menu, LV_DROPDOWN_DIR_LEFT);

    lv_dropdown_set_show_selected(local->ctx_menu, false);
    lv_dropdown_set_text(local->ctx_menu, "");
    lv_dropdown_set_symbol(local->ctx_menu, NULL);
    lv_dropdown_clear_options(local->ctx_menu);
    lv_dropdown_add_option(local->ctx_menu, LV_SYMBOL_PLUS " Add widget", 0);
    lv_dropdown_add_option(local->ctx_menu, LV_SYMBOL_REFRESH " Change widget", 1);
    lv_dropdown_add_option(local->ctx_menu, LV_SYMBOL_CLOSE " Remove widget", 2);

    local->dummy = lv_obj_create(par, NULL);
    lv_obj_set_size(local->dummy, 0, 0);
    lv_obj_set_event_cb(local->dummy, page_edit_event_cb);

    gui_set_loop_speed(50);

    local->page.number_of_widgets = 0;

	return par;
}

static void page_edit_loop()
{
    widgets_update(&local->page);
}

static void page_edit_stop()
{
    lv_style_reset(&local->label_style);

	widgets_deinit_page(&local->page);
}
