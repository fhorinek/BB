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
#include "gui/widgets/widget_flags.h"
#include "gui/dialog.h"
#include "gui/anim.h"

#include "gui/ctx.h"

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
    lv_obj_t * selector;

    lv_obj_t * grid;
    lv_point_t * grid_points;

    uint8_t focus_index;

    lv_obj_t * par;
    bool changed;

    lv_style_t label_style;
    lv_style_t border_style;

	char page_name[PAGE_NAME_LEN + 1];
	uint8_t page_index;
);

#define PAGE_EDIT_ADD_NEW   0xFF

void page_edit_set_focus(uint8_t index);

void page_edit_set_selector_mode()
{
	if (local->selector == NULL)
		return;

    if (local->mode == mode_select)
    	lv_obj_set_style_local_border_color(local->selector, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);
    else if(local->mode == mode_move_x || local->mode == mode_move_y)
    	lv_obj_set_style_local_border_color(local->selector, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
    else
    	lv_obj_set_style_local_border_color(local->selector, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);

}

void page_edit_set_mode(page_edit_mode_t mode)
{
    local->mode = mode;

    if (local->page.number_of_widgets == 0)
    {
        lv_label_set_text(local->mode_label, LV_SYMBOL_PLUS " Add widget");
        ctx_hide();
        return;
    }

    ctx_show();

    page_edit_set_selector_mode();


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
        default:
		break;
    }
}

void page_edit_recreate_widget(widget_slot_t * ws)
{
    widget_deinit(ws);

    lv_obj_del(ws->obj);
    local->selector = NULL;

    widget_init(ws, local->page.base);
    lv_obj_add_style(ws->obj, LV_OBJ_PART_MAIN, &local->border_style);
    page_edit_set_focus(local->focus_index);
}

void page_edit_move_widget(widget_slot_t * ws, int8_t dir)
{
    int16_t inc = WIDGET_GRID * dir;

    int16_t w = lv_obj_get_width(local->par);
    int16_t h = lv_obj_get_height(local->par);

    local->changed = true;


    bool scale = false;
    switch(local->mode)
    {
        case(mode_move_x):
            ws->x = max(ws->x + inc, 0);
            if (ws->x > w - ws->w)
            {
                ws->w = max(w - ws->x, ws->widget->w_min);
                ws->x = w - ws->w;
                scale = true;
            }
            else
            {
            	lv_obj_set_pos(ws->obj, ws->x, ws->y);
            }
        break;
        case(mode_move_y):
            ws->y = max(ws->y + inc, 0);
            if (ws->y > h - ws->h)
            {
                ws->h = max(h - ws->y, ws->widget->h_min);
                ws->y = h - ws->h;
                scale = true;
            }
            else
            {
            	lv_obj_set_pos(ws->obj, ws->x, ws->y);
            }
        break;
        case(mode_size_x):
            scale = true;
            ws->w = max(ws->w + inc, ws->widget->w_min);
            ws->w = min(ws->w, w);
            if (ws->w + ws->x > w)
                ws->x = w - ws->w;

        break;
        case(mode_size_y):
            scale = true;
            ws->h = max(ws->h + inc, ws->widget->h_min);
            ws->h = min(ws->h, h);
            if (ws->h + ws->y > h)
                ws->y = h - ws->h;

        break;
        default:
		break;
    }

    if (scale)
    	page_edit_recreate_widget(ws);
}

void page_edit_remove_cb(dialog_result_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        anim_fade_out_delete(local->page.widget_slots[local->focus_index].obj);
        widgets_remove(&local->page, local->focus_index);

        local->changed = true;
        local->selector = NULL;
        page_edit_set_focus(local->focus_index - 1);

        //if there are no widgets -> show message
        if (local->page.number_of_widgets == 0)
        {
            widgets_add_page_empty_label(&local->page);
        }
        page_edit_set_mode(mode_select);
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


void page_edit_set_focus(uint8_t index)
{
	if (local->page.number_of_widgets == 0)
		return;

	if (index >= local->page.number_of_widgets)
		index = 0;


	lv_obj_t * obj = local->page.widget_slots[index].obj;
	local->focus_index = index;

	if (local->selector != NULL)
		lv_obj_del(local->selector);

	local->selector = lv_obj_create(obj, NULL);

	lv_coord_t w = lv_obj_get_width(obj);
	lv_coord_t h = lv_obj_get_height(obj);

	lv_obj_set_pos(local->selector, 0, 0);
	lv_obj_set_size(local->selector, w, h);

	lv_obj_set_style_local_bg_opa(local->selector, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_width(local->selector, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_border_opa(local->selector, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
	page_edit_set_selector_mode();

	lv_obj_move_foreground(obj);
}

void page_edit_ctx_open(uint8_t index)
{
	ctx_clear();
	ctx_add_option(LV_SYMBOL_PLUS " Add widget");
	ctx_add_option(LV_SYMBOL_REFRESH " Change widget");
	ctx_add_option(LV_SYMBOL_TRASH " Remove widget");

	bool widget_flags_title = false;
	//add flag options
	for (uint8_t i = 0; i < wf_def_size; i++)
	{
		if (local->page.widget_slots[local->focus_index ].widget->flags & _b(i))
		{
			if (!widget_flags_title)
			{
				widget_flags_title = true;
				ctx_add_option("----Widget settings----");
			}

			if (widget_flag_is_set(&local->page.widget_slots[local->focus_index], i))
				ctx_add_option(widgets_flags[i].on_text);
			else
				ctx_add_option(widgets_flags[i].off_text);
		}
	}

	ctx_open(index);
}

static void page_edit_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_DELETE)
        return;

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
        	if (local->page.number_of_widgets > 0)
        	{
                switch (key)
                {
                    case(LV_KEY_RIGHT):
                    	if (local->mode == mode_select)
                    		page_edit_set_focus((local->focus_index + 1) % local->page.number_of_widgets);
                    	else
                    		page_edit_move_widget(&local->page.widget_slots[local->focus_index ], +1);
                    break;

                    case(LV_KEY_LEFT):
						if (local->mode == mode_select)
							page_edit_set_focus((local->focus_index + local->page.number_of_widgets - 1) % local->page.number_of_widgets);
						else
							page_edit_move_widget(&local->page.widget_slots[local->focus_index ], -1);
                    break;

                    case(LV_KEY_HOME):
						page_edit_ctx_open(0);
                    break;
                }
        	}

        }
		break;
    }

}

bool page_edit_ctx_cb(uint8_t option, lv_obj_t * last_focus)
{
    if (option != CTX_CANCEL)
    {
        //add
        if (option == 0)
        {
            page_edit_go_to_widget_list(PAGE_EDIT_ADD_NEW);
        }
        //change
        else if (option == 1)
        {
            page_edit_go_to_widget_list(local->focus_index);
        }
        //remove
        else if (option == 2)
        {
            char text[128];
            snprintf(text, sizeof(text), "Do you really want to remove widget '%s'?", local->page.widget_slots[local->focus_index].widget->name);
            dialog_show("Remove widget", text, dialog_yes_no, page_edit_remove_cb);
        }
        else
        {
			uint8_t f_index = 4;

			// *** Flags ***
			for (uint8_t i = 0; i < wf_def_size; i++)
			{
				if (local->page.widget_slots[local->focus_index].widget->flags & _b(i))
				{
					if (f_index == option)
					{
						if (widget_flag_is_set(&local->page.widget_slots[local->focus_index], i))
							widget_flag_unset(&local->page.widget_slots[local->focus_index], i);
						else
							widget_flag_set(&local->page.widget_slots[local->focus_index], i);

						//update widget
						page_edit_recreate_widget(&local->page.widget_slots[local->focus_index]);
                        local->changed = true;
						break;
					}
					f_index++;
				}
			}
			page_edit_ctx_open(option);
			return false;
        }

    }
    else
    {
        //refocus old widget
//        page_edit_set_mode(mode_select);
//        lv_group_focus_obj(local->page.widget_slots[local->focus_index].obj);
    }

    return true;
}

void page_edit_set_page_name(char * filename, uint8_t index)
{
    strncpy(local->page_name, filename, sizeof(local->page_name));
    widgets_load_from_file(&local->page, filename);
    widgets_sort_page(&local->page);
    widgets_init_page(&local->page, local->par);

    for (uint8_t i = 0; i < local->page.number_of_widgets; i++)
    {
    	lv_obj_add_style(local->page.widget_slots[i].obj, LV_OBJ_PART_MAIN, &local->border_style);
    }

    local->page_index = index;

	page_edit_set_focus(0);
    page_edit_set_mode(mode_select);
}

void page_edit_modify_widget(widget_t * w, uint8_t widget_index)
{
    local->changed = true;

    if (widget_index == PAGE_EDIT_ADD_NEW)
    {
        if (local->page.number_of_widgets == 0)
        {
            //remove empty label
            lv_obj_del(lv_obj_get_child(local->page.base, NULL));
        }

        widgets_add(&local->page, w);
        page_edit_set_focus(local->page.number_of_widgets - 1);

        int16_t w = lv_obj_get_width(local->par);
        int16_t h = lv_obj_get_height(local->par);

        widget_slot_t * new_ws = &local->page.widget_slots[local->page.number_of_widgets - 1];

        if (local->page.number_of_widgets > 1)
        {
            widget_slot_t * last_ws = &local->page.widget_slots[local->page.number_of_widgets - 2];
            if (w - last_ws->x - last_ws->w >= new_ws->w)
            {
                new_ws->x = last_ws->x + last_ws->w;
                new_ws->y = last_ws->y;
                lv_obj_set_pos(new_ws->obj, new_ws->x, new_ws->y);
            }
            else if (h - last_ws->y - last_ws->h >= new_ws->h)
            {
                new_ws->x = 0;
                new_ws->y = last_ws->y + last_ws->h;
                lv_obj_set_pos(new_ws->obj, new_ws->x, new_ws->y);
            }
        }
    }
    else
    {
        lv_obj_del(local->page.widget_slots[widget_index].obj);
        widgets_change(&local->page, widget_index, w);
        page_edit_set_focus(widget_index);
    }

    lv_obj_add_style(local->page.widget_slots[local->focus_index].obj, LV_OBJ_PART_MAIN, &local->border_style);
    page_edit_set_mode(mode_select);
}



static lv_obj_t * page_edit_init(lv_obj_t * par)
{
    local->par = par;
    local->changed = false;

    //Grid
    lv_obj_t * grid = lv_line_create(par, NULL);
    lv_obj_set_pos(grid, 0, 0);
    lv_obj_set_size(grid, LV_HOR_RES, LV_VER_RES - GUI_STATUSBAR_HEIGHT);

    uint8_t grid_points_hor = 2 * (LV_HOR_RES / WIDGET_GRID);
    uint8_t grid_points_ver = 2 * (LV_VER_RES / WIDGET_GRID);
    local->grid_points = malloc(sizeof(lv_point_t) * (grid_points_hor + grid_points_ver));

    for (uint8_t x = 0; x < grid_points_hor / 2; x++)
    {
    	uint8_t a = (x % 2);
		uint8_t b = !(x % 2);

    	local->grid_points[x * 2 + a].x = x * WIDGET_GRID;
    	local->grid_points[x * 2 + b].x = x * WIDGET_GRID;
    	local->grid_points[x * 2 + a].y = -1;
    	local->grid_points[x * 2 + b].y = LV_VER_RES - GUI_STATUSBAR_HEIGHT;
    }

    for (uint8_t y = 0; y < grid_points_ver / 2; y++)
    {
    	uint8_t a = (y % 2);
		uint8_t b = !(y % 2);

    	local->grid_points[grid_points_hor + y * 2 + a].y = y * WIDGET_GRID;
    	local->grid_points[grid_points_hor + y * 2 + b].y = y * WIDGET_GRID;
    	local->grid_points[grid_points_hor + y * 2 + a].x = -1;
    	local->grid_points[grid_points_hor + y * 2 + b].x = LV_HOR_RES;
    }

    lv_line_set_points(grid, local->grid_points, grid_points_hor + grid_points_ver);
    lv_obj_set_style_local_line_color(grid, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_line_width(grid, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_line_opa(grid, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_10);
    local->grid = grid;

    //border style
    lv_style_init(&local->border_style);
    lv_style_set_border_color(&local->border_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_border_width(&local->border_style, LV_STATE_DEFAULT, 1);

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

    lv_obj_move_foreground(local->mode_label);

    //selector
    local->selector = NULL;

    //ctx menu
    ctx_show();
    ctx_set_cb(page_edit_ctx_cb);

    //dummy cb
    gui_set_dummy_event_cb(par, page_edit_event_cb);

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
   lv_style_reset(&local->border_style);

   free(local->grid_points);

	widgets_deinit_page(&local->page);
}
