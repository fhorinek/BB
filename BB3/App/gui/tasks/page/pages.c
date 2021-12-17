/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include <gui/tasks/menu/settings.h>
#include "pages.h"

#include "page_settings.h"

#include "gui/widgets/pages.h"

#include "gui/gui_list.h"
#include "gui/statusbar.h"
#include "gui/dialog.h"

#include "gui/widgets/widgets.h"

#include "drivers/power/led.h"

#include "etc/bootloader.h"

extern const lv_img_dsc_t tile;

#define MENU_TIMEOUT	2000
#define EDIT_TIMEOUT	6000
#define OVER_TIMEOUT	10000

#define MENU_RADIUS	15
#define MENU_WIDTH	35
#define MENU_HEIGHT	180

//States
#define MENU_IDLE			0
#define MENU_IN				1
#define MENU_SHOW			2
#define MENU_OUT			3
#define MENU_EDIT_WIDGET    5
#define MENU_EDIT_OVERLAY   6

#define SPLASH_IN			7
#define SPLASH_OUT			8

#define PAGE_SWITCH_LEFT	9
#define PAGE_SWITCH_RIGHT	10


#define PAGE_ANIM_FROM_LEFT		-1
#define PAGE_ANIM_NONE			0
#define PAGE_ANIM_FROM_RIGHT	1

#define MENU_ANIM_TIME		500
#define SPLASH_ANIM_TIME	1000

REGISTER_TASK_ILS(pages,
	//on / off mask
	lv_obj_t * mask;
	lv_objmask_mask_t * mask_param;

	//menu
	lv_obj_t * left_menu;
	lv_obj_t * right_menu;
	lv_obj_t * center_menu;

	lv_obj_t * butt_layout;
	lv_obj_t * butt_short1;
	lv_obj_t * butt_power;
	lv_obj_t * butt_short2;
	lv_obj_t * butt_settings;

	lv_obj_t * selector;

	//indicator
	lv_obj_t * indicator;
	lv_obj_t * label;

	lv_style_t menu_style;

	widget_slot_t * active_widget;

	//animation
	uint8_t state;
	lv_anim_t anim;
	uint32_t timer;

	//pages
	page_layout_t * page;
	page_layout_t * page_old;

	uint8_t pages_cnt;
	uint8_t actual_page;

	uint8_t pwr_off_button_cnt;

	char page_name[PAGE_NAME_LEN + 1];
);

void pages_load(char * filename, int8_t anim);


void page_focus_widget(lv_obj_t * obj)
{
    if (local->selector != NULL)
    {
        lv_obj_del(local->selector);
        local->selector = NULL;
    }

    if (obj != NULL)
    {
        local->selector = lv_obj_create(obj, NULL);

        lv_coord_t w = lv_obj_get_width(obj);
        lv_coord_t h = lv_obj_get_height(obj);

        lv_obj_set_pos(local->selector, 0, 0);
        lv_obj_set_size(local->selector, w, h);

        lv_obj_set_style_local_bg_opa(local->selector, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_obj_set_style_local_border_width(local->selector, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);
        lv_obj_set_style_local_border_color(local->selector, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);
        lv_obj_set_style_local_border_opa(local->selector, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);

        lv_obj_move_foreground(obj);
    }
}

void gui_page_set_mode(cfg_entry_t * cfg)
{
	char * name = config_get_text(cfg);

	if (strlen(name) == 0)
		return;

	uint8_t i;
	for (i = 0; i < PAGE_MAX_COUNT; i++)
	{
		if (strcmp(name, config_get_text(&profile.ui.page[i])) == 0)
			break;
	}

	if (i == PAGE_MAX_COUNT)
	{
		INFO("Unable to switch to '%s'", name);
		return;
	}

	if (gui.task.actual == &gui_pages)
	{
		INFO("Switching to page '%s' active", name);

		if (i == local->actual_page)
			return;

		gui_lock_acquire();

		uint8_t last_page = local->actual_page;
		local->actual_page = i;

		uint8_t anim = PAGE_ANIM_NONE;
		if (config_get_bool(&config.display.page_anim))
		{
			anim = (last_page >local->actual_page) ? PAGE_ANIM_FROM_LEFT : PAGE_ANIM_FROM_RIGHT;
		}

		local->state = MENU_IDLE;
		local->active_widget = NULL;
		page_focus_widget(NULL);

		pages_load((char *)pages_get_name(local->actual_page), anim);


		char * label = NULL;
		if (cfg == &profile.ui.autoset.take_off)
			label = "Take off";
		else if (cfg == &profile.ui.autoset.circle)
			label = "Circling";
		else if (cfg == &profile.ui.autoset.glide)
			label = "Glide";
		else if (cfg == &profile.ui.autoset.land)
			label = "Landing";
		else if (cfg == &profile.ui.autoset.power_on)
			label = "Power on";


		if (label != NULL)
		{
			lv_label_set_text(local->label, label);
			lv_obj_fade_in(local->label, GUI_INDICATOR_ANIM, 0);
			lv_obj_fade_out(local->label, GUI_INDICATOR_ANIM, GUI_LABEL_DELAY);
		}

		gui_lock_release();
	}
	else
	{
		INFO("Switching to page '%s' pasive", name);

		config_set_int(&profile.ui.page_last, i);
	}
}

void pages_menu_anim_cb(void * obj, lv_anim_value_t val)
{
	lv_style_set_bg_opa(&local->menu_style, LV_STATE_DEFAULT, val * 5);
	lv_obj_set_x(local->left_menu, val - MENU_RADIUS - MENU_WIDTH);
	lv_obj_set_x(local->right_menu, LV_HOR_RES - val);

	lv_obj_set_width(local->center_menu, LV_HOR_RES - val * 2);
	lv_obj_set_pos(local->center_menu, val, LV_VER_RES - val - GUI_STATUSBAR_HEIGHT);
}

void pages_splash_anim_cb(void * obj, lv_anim_value_t val)
{
	int16_t w, h, r;

	w = lv_obj_get_width(local->mask);
	h = lv_obj_get_height(local->mask);

	r = min(w / 2, (h / 2) - val);

	lv_area_t a;
	a.x1 = (w / 2) - r;
	a.y1 = val;
	a.x2 = (w / 2) + r;
	a.y2 = h - val;

	if (local->mask_param != NULL)
		lv_objmask_remove_mask(local->mask, local->mask_param);

	lv_draw_mask_radius_param_t mask_param;
	lv_draw_mask_radius_init(&mask_param, &a, val, false);
	local->mask_param = lv_objmask_add_mask(local->mask, &mask_param);
}


void pages_menu_show()
{
    lv_obj_set_hidden(local->butt_power, false);
	lv_anim_set_time(&local->anim, MENU_ANIM_TIME);
	lv_anim_set_exec_cb(&local->anim, pages_menu_anim_cb);
	lv_anim_set_values(&local->anim, 0, MENU_WIDTH);
	lv_anim_start(&local->anim);
	local->state = MENU_IN;
}

void pages_menu_hide()
{
	lv_anim_set_time(&local->anim, MENU_ANIM_TIME);
	lv_anim_set_exec_cb(&local->anim, pages_menu_anim_cb);
	lv_anim_set_values(&local->anim, MENU_WIDTH, 0);
	lv_anim_start(&local->anim);
	local->state = MENU_OUT;
}

void pages_splash_show()
{
	lv_anim_set_time(&local->anim, SPLASH_ANIM_TIME);
	lv_anim_set_exec_cb(&local->anim, pages_splash_anim_cb);

	lv_anim_set_values(&local->anim, lv_obj_get_height(local->mask) / 2, 0);
	local->mask_param = NULL;

	lv_anim_start(&local->anim);
	local->state = SPLASH_IN;

	if (file_exists(PATH_NEW_FW))
	{
		f_unlink(PATH_NEW_FW);

		gui_show_release_note();

	    if (bootloader_update(PATH_BL_FW_AUTO) == bl_update_ok)
	    {
	        statusbar_msg_add(STATUSBAR_MSG_INFO, "Bootloader successfully updated!");
	    }
	    f_unlink(PATH_BL_FW_AUTO);
	}
}

void pages_splash_hide()
{
	lv_anim_set_time(&local->anim, SPLASH_ANIM_TIME);
	lv_anim_set_exec_cb(&local->anim, pages_splash_anim_cb);

	lv_anim_set_values(&local->anim, 0, lv_obj_get_height(local->mask) / 2);
	local->mask_param = NULL;

	lv_anim_start(&local->anim);
	local->state = SPLASH_OUT;

	led_set_backlight(0);
}

void pages_indicator_show()
{
	lv_obj_t * obj = NULL;

	for (uint8_t i = 0; i < local->pages_cnt; i++)
	{
		obj = lv_obj_get_child_back(local->indicator, obj);

		if (i == local->actual_page)
			lv_led_on(obj);
		else
			lv_led_off(obj);
	}

	lv_obj_fade_in(local->indicator, GUI_INDICATOR_ANIM, 0);
	lv_obj_fade_out(local->indicator, GUI_INDICATOR_ANIM, GUI_INDICATOR_DELAY);
}

void pages_anim_ready_cb(lv_anim_t * a)
{
	(void)a;

	if (local->state == MENU_IN)
	{
		local->state = MENU_SHOW;
		local->timer = HAL_GetTick() + MENU_TIMEOUT;
		return;
	}

	if (local->state == SPLASH_IN)
	{
		if (local->mask_param != NULL)
			lv_objmask_remove_mask(local->mask, local->mask_param);

		local->state = MENU_IDLE;
		return;
	}

	if (local->state == SPLASH_OUT)
	{
		if (local->mask_param != NULL)
			lv_objmask_remove_mask(local->mask, local->mask_param);

		pages_stop();
		gui_stop();
		system_poweroff();

		return;
	}

	if (local->state == MENU_OUT)
	{
		local->state = MENU_IDLE;
		return;
	}

	if (local->state == PAGE_SWITCH_LEFT || local->state == PAGE_SWITCH_RIGHT)
	{
		if (local->page_old != NULL)
		{
			widgets_deinit_page(local->page_old);

			lv_obj_del(local->page_old->base);
			free(local->page_old);
			local->page_old = NULL;
		}
		local->state = MENU_IDLE;

		return;
	}
}

void pages_power_off()
{
	pages_splash_hide();
	statusbar_hide();
}

void pages_create_menu(lv_obj_t * base)
{
	lv_style_init(&local->menu_style);
	lv_style_set_bg_opa(&local->menu_style, LV_STATE_DEFAULT, LV_OPA_50);
	lv_style_set_bg_color(&local->menu_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_style_set_text_color(&local->menu_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_radius(&local->menu_style, LV_STATE_DEFAULT, MENU_RADIUS);

	local->left_menu = lv_cont_create(base, NULL);
	lv_obj_add_style(local->left_menu, LV_CONT_PART_MAIN, &local->menu_style);
	lv_obj_set_pos(local->left_menu, -MENU_RADIUS - MENU_WIDTH, LV_VER_RES - MENU_HEIGHT - GUI_STATUSBAR_HEIGHT);
	lv_obj_set_size(local->left_menu, MENU_WIDTH + MENU_RADIUS, MENU_HEIGHT + MENU_RADIUS);

	local->butt_layout = lv_label_create(local->left_menu, NULL);
	lv_label_set_text(local->butt_layout, LV_SYMBOL_LIST);
	lv_obj_align_origo(local->butt_layout, NULL, LV_ALIGN_IN_TOP_MID, MENU_RADIUS / 2, MENU_HEIGHT / 4);

	//TODO: shortcut 1
	local->butt_short1 = lv_label_create(local->left_menu, NULL);
	lv_label_set_text(local->butt_short1, "");
	lv_obj_align_origo(local->butt_short1, NULL, LV_ALIGN_CENTER, MENU_RADIUS / 2, MENU_HEIGHT / 5);


	local->right_menu = lv_cont_create(base, NULL);
	lv_obj_add_style(local->right_menu, LV_CONT_PART_MAIN, &local->menu_style);
	lv_obj_set_pos(local->right_menu, LV_HOR_RES, LV_VER_RES - MENU_HEIGHT - GUI_STATUSBAR_HEIGHT);
	lv_obj_set_size(local->right_menu, MENU_WIDTH + MENU_RADIUS, MENU_HEIGHT + MENU_RADIUS);

	local->butt_settings = lv_label_create(local->right_menu, NULL);
	lv_label_set_text(local->butt_settings, LV_SYMBOL_SETTINGS);
	lv_obj_align_origo(local->butt_settings, NULL, LV_ALIGN_IN_TOP_MID, -MENU_RADIUS / 2, MENU_HEIGHT / 4);

	//TODO: shortcut 2
	local->butt_short2 = lv_label_create(local->right_menu, NULL);
	lv_label_set_text(local->butt_short2, "");
	lv_obj_align_origo(local->butt_short2, NULL, LV_ALIGN_CENTER, -MENU_RADIUS / 2, MENU_HEIGHT / 5);


	local->center_menu = lv_cont_create(base, NULL);
	lv_obj_add_style(local->center_menu, LV_CONT_PART_MAIN, &local->menu_style);
	lv_obj_set_pos(local->center_menu, 0, LV_VER_RES - GUI_STATUSBAR_HEIGHT);
	lv_obj_set_size(local->center_menu, LV_HOR_RES, MENU_WIDTH);
	lv_obj_set_style_local_radius(local->center_menu, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_cont_set_layout(local->center_menu, LV_LAYOUT_CENTER);

	local->butt_power = lv_label_create(local->center_menu, NULL);
	lv_label_set_text(local->butt_power, LV_SYMBOL_POWER);

	lv_anim_init(&local->anim);
	lv_anim_set_ready_cb(&local->anim, pages_anim_ready_cb);

	local->indicator = lv_cont_create(base, NULL);
	lv_obj_set_style_local_bg_opa(local->indicator, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_pad_hor(local->indicator, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, GUI_INDICATOR_HEIGHT);

	lv_cont_set_fit(local->indicator, LV_FIT_TIGHT);
	lv_cont_set_layout(local->indicator, LV_LAYOUT_ROW_MID);
	lv_obj_set_style_local_pad_inner(local->indicator, LV_LED_PART_MAIN, LV_STATE_DEFAULT, GUI_INDICATOR_DOT_SIZE / 2);

	for (uint8_t i = 0; i < local->pages_cnt; i++)
	{
		lv_obj_t * dot = lv_led_create(local->indicator, NULL);
		lv_obj_set_size(dot, GUI_INDICATOR_DOT_SIZE, GUI_INDICATOR_DOT_SIZE);


		lv_led_off(dot);
		if (i == local->actual_page)
			lv_led_on(dot);
	}

	lv_obj_align(local->indicator, NULL, LV_ALIGN_OUT_TOP_MID, 0, GUI_INDICATOR_Y_POS);

	local->label = lv_label_create(base, NULL);
	lv_obj_align(local->label, local->indicator, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
	lv_obj_set_style_local_bg_color(local->label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_style_local_bg_opa(local->label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
	lv_obj_set_style_local_text_color(local->label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_obj_set_style_local_radius(local->label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 10);
	lv_obj_set_style_local_pad_all(local->label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 10);
	lv_obj_set_auto_realign(local->label, true);
	lv_obj_set_style_local_opa_scale(local->label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	local->state = MENU_IDLE;
}

void pages_lock_reset()
{
	local->timer = HAL_GetTick() + OVER_TIMEOUT;
}

void pages_lock_widget()
{
	local->state = MENU_EDIT_OVERLAY;
	local->timer = HAL_GetTick() + OVER_TIMEOUT;
}

void pages_unlock_widget()
{
	local->state = MENU_EDIT_WIDGET;
	local->timer = HAL_GetTick() + MENU_TIMEOUT;
}

static void pages_event_cb(lv_obj_t * obj, lv_event_t event)
{
    switch(event)
    {
        case LV_EVENT_LONG_PRESSED_REPEAT:
        {
            if (local->state == MENU_SHOW)
            {
                local->timer = HAL_GetTick() + MENU_TIMEOUT;
                local->pwr_off_button_cnt++;
                if (local->pwr_off_button_cnt > 2)
                    lv_obj_set_hidden(local->butt_power, local->pwr_off_button_cnt % 2);
                if (local->pwr_off_button_cnt > 15)
                    pages_power_off();
            }
        }
        break;

        case LV_EVENT_RELEASED:
        {
            if (local->state == MENU_SHOW)
            {
                lv_obj_set_hidden(local->butt_power, false);
            }
            break;
        }

        case LV_EVENT_LONG_PRESSED:
        {
        	if (local->state == MENU_IDLE)
        	{
        		pages_menu_show();
                local->pwr_off_button_cnt = 0;
        	}
        	else if (local->state == MENU_EDIT_WIDGET)
        	{
        	    widgets_edit(local->active_widget, WIDGET_ACTION_HOLD);
        	}
        	else if (local->state == MENU_EDIT_OVERLAY)
        	{
        		widgets_edit(local->active_widget, WIDGET_ACTION_CLOSE);
        	}

        }
        break;

        case LV_EVENT_SHORT_CLICKED:
            if (local->state == MENU_SHOW)
            {
                pages_power_off();
            }
            else
            {
                //edit mode
                if (local->state == MENU_IDLE || local->state == MENU_EDIT_WIDGET)
                {
                    if (widgets_editable(local->page))
                    {
                        local->state = MENU_EDIT_WIDGET;
                        local->timer = HAL_GetTick() + MENU_TIMEOUT;

                        //defocus old
                        if (local->active_widget != NULL)
                        {
                        	page_focus_widget(NULL);
                        	widgets_edit(local->active_widget, WIDGET_ACTION_DEFOCUS);
                        }

                        local->active_widget = widgets_editable_select_next(local->page, local->active_widget);

                        //focus new
                        if (local->active_widget != NULL)
                            page_focus_widget(local->active_widget->obj);
                        else
                        	local->state = MENU_IDLE;
                    }
                }
                else if (local->state == MENU_EDIT_OVERLAY)
                {
                	widgets_edit(local->active_widget, WIDGET_ACTION_MIDDLE);
                }

            }

		break;

        case LV_EVENT_KEY:
        {
        	uint32_t key = *((uint32_t *) lv_event_get_data());
        	if (local->state == MENU_SHOW)
        	{
        		switch (key)
        		{
    			case(LV_KEY_RIGHT):
    				gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    			break;
    			case(LV_KEY_LEFT):
    				gui_switch_task(&gui_page_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    				page_settings_set_page_name(local->page_name, local->actual_page);
    			break;
        		}
        	}
        	else if (local->state == MENU_IDLE)
        	{
        		if (local->pages_cnt == 1)
        		{
        			pages_indicator_show();
        			break;
        		}


        		switch (key)
        		{
    			case(LV_KEY_RIGHT):
    				local->actual_page = (local->actual_page + 1) % local->pages_cnt;
					pages_load((char *)pages_get_name(local->actual_page),
							config_get_bool(&config.display.page_anim) ? PAGE_ANIM_FROM_RIGHT : PAGE_ANIM_NONE);

    			break;
    			case(LV_KEY_LEFT):
   					local->actual_page = ((local->actual_page > 0) ? local->actual_page : local->pages_cnt) - 1;
					pages_load((char *)pages_get_name(local->actual_page),
							config_get_bool(&config.display.page_anim) ? PAGE_ANIM_FROM_LEFT : PAGE_ANIM_NONE);
    			break;


        		}
        	}
        	else if (local->state == MENU_EDIT_WIDGET || local->state == MENU_EDIT_OVERLAY)
        	{
                switch (key)
                {
					case(LV_KEY_RIGHT):
						widgets_edit(local->active_widget, WIDGET_ACTION_RIGHT);
					break;
					case(LV_KEY_LEFT):
						widgets_edit(local->active_widget, WIDGET_ACTION_LEFT);
					break;
					case(LV_KEY_HOME):
						widgets_edit(local->active_widget, WIDGET_ACTION_MENU);
					break;

					case(LV_KEY_ESC):
						if (local->state == MENU_EDIT_OVERLAY)
						{
							widgets_edit(local->active_widget, WIDGET_ACTION_CLOSE);
						}
						else
						{
							local->state = MENU_IDLE;
							if (local->active_widget != NULL)
							{
								widgets_edit(local->active_widget, WIDGET_ACTION_DEFOCUS);
								page_focus_widget(NULL);
							}
							local->active_widget = NULL;
						}
					break;
                }
        	}
        }
		break;
    }

}

void pages_switch_anim_cb(void * obj, lv_anim_value_t val)
{
	lv_obj_set_x(local->page->base, val);
	if (local->state == PAGE_SWITCH_RIGHT)
	{
		lv_obj_set_x(local->page_old->base, val - LV_HOR_RES);
	}
	else
	{
		lv_obj_set_x(local->page_old->base, val + LV_HOR_RES);
	}

}

void pages_load(char * filename, int8_t anim)
{
	if (local->page != NULL)
	{
		//switch actual page to old
		local->page_old = local->page;
	}

	//create new base layer for widgets
	local->page = (page_layout_t *) malloc(sizeof(page_layout_t));

	if (anim != PAGE_ANIM_NONE)
	{
		//create anim
		lv_anim_set_values(&local->anim, LV_HOR_RES * anim, 0);
		lv_anim_set_exec_cb(&local->anim, pages_switch_anim_cb);
		lv_anim_start(&local->anim);
		if (anim == PAGE_ANIM_FROM_LEFT)
			local->state = PAGE_SWITCH_LEFT;
		else
			local->state = PAGE_SWITCH_RIGHT;

	}
	else
	{
		//no animation just delete old page
		if (local->page_old != NULL)
		{
		    widgets_deinit_page(local->page_old);

			lv_obj_del(local->page_old->base);
			free(local->page_old);
			local->page_old = NULL;
		}
	}

	pages_indicator_show();

	strncpy(local->page_name, filename, sizeof(local->page_name));
	if (widgets_load_from_file(local->page, filename))
    {
	    //page loaded
        widgets_init_page(local->page, local->mask);
    }
	else
	{
	    //page not found
	    widgets_create_base(local->page, local->mask);
		lv_obj_t * label = lv_label_create(local->page->base, NULL);
		lv_label_set_text_fmt(label, "Page '%s'\nnot found.", filename);
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
		lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	}


	lv_obj_move_background(local->page->base);
}

static lv_obj_t * pages_init(lv_obj_t * par)
{
    if (gui.task.last != NULL || gui.task.last == &gui_pages)
        config_store_all();

    local->selector = NULL;

	lv_obj_set_style_local_bg_color(par, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

	gui_set_dummy_event_cb(par, pages_event_cb);

	local->mask = lv_objmask_create(par, NULL);
	lv_obj_set_pos(local->mask, 0, 0);
	lv_obj_set_size(local->mask, lv_obj_get_width(par), lv_obj_get_height(par));

	local->page = NULL;
	local->page_old = NULL;

	local->pages_cnt = pages_get_count();

	char path[PATH_LEN];
	snprintf(path, sizeof(path), "%s/%s", PATH_PAGES_DIR, config_get_text(&config.flight_profile));
	if (!file_exists(path) || local->pages_cnt == 0)
	{
	    //copy default layouts
		char def[PATH_LEN];
		snprintf(def, sizeof(def), "%s/defaults/pages", PATH_ASSET_DIR);
		copy_dir(def, path);

		//set default pages
        config_set_text(&profile.ui.page[0], "basic");
        config_set_text(&profile.ui.page[1], "thermal");

        config_set_text(&profile.ui.autoset.take_off, "basic");
        config_set_text(&profile.ui.autoset.circle, "thermal");
        config_set_text(&profile.ui.autoset.glide, "basic");
        local->pages_cnt = 3;
	}

	local->actual_page = config_get_int(&profile.ui.page_last);
	if (local->actual_page >= local->pages_cnt)
	{
		local->actual_page = 0;
		config_set_int(&profile.ui.page_last, 0);
	}

	pages_create_menu(local->mask);

	pages_load(pages_get_name(local->actual_page), PAGE_ANIM_NONE);

	local->state = MENU_IDLE;
	local->active_widget = NULL;

	gui_set_loop_speed(50);

	return par;
}

static void pages_loop()
{
	if (local->state == MENU_SHOW && local->timer < HAL_GetTick())
	{
		pages_menu_hide();
	}

	if (local->state == MENU_EDIT_WIDGET && local->timer < HAL_GetTick())
	{
        //defocus old
        if (local->active_widget != NULL)
        {
//        	local->active_widget->obj->signal_cb(local->active_widget->obj, LV_SIGNAL_DEFOCUS, NULL);
        	page_focus_widget(NULL);
        	widgets_edit(local->active_widget, WIDGET_ACTION_DEFOCUS);
        	local->active_widget = NULL;
        }

		local->state = MENU_IDLE;
	}

	if (local->state == MENU_EDIT_OVERLAY && local->timer < HAL_GetTick())
	{
        //defocus old
        if (local->active_widget != NULL)
        {
//        	local->active_widget->obj->signal_cb(local->active_widget->obj, LV_SIGNAL_DEFOCUS, NULL);
        	page_focus_widget(NULL);
        	widgets_edit(local->active_widget, WIDGET_ACTION_DEFOCUS);

        	//close overlay
    		widgets_edit(local->active_widget, WIDGET_ACTION_CLOSE);
        	local->active_widget = NULL;
        }

		local->state = MENU_IDLE;
	}
	widgets_update(local->page);
}

static void pages_stop()
{
	lv_style_reset(&local->menu_style);

	widgets_deinit_page(local->page);

	config_set_int(&profile.ui.page_last, local->actual_page);

	free(local->page);
}
