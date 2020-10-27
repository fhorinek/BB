/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "pages.h"

#include "settings.h"
#include "page_settings.h"

#include "../gui_list.h"
#include "../map/tile.h"
#include "../gui/statusbar.h"

#include "../widgets/widgets.h"
#include "../../config/config.h"

extern const lv_img_dsc_t tile;

#define MENU_TIMEOUT	2000

#define MENU_RADIUS	15
#define MENU_WIDTH	35
#define MENU_HEIGHT	180

//States
#define MENU_IDLE			0
#define MENU_IN				1
#define MENU_SHOW			2
#define MENU_OUT			3

#define SPLASH_IN			4
#define SPLASH_OUT			5

#define PAGE_SWITCH_LEFT	6
#define PAGE_SWITCH_RIGHT	7


#define PAGE_ANIM_FROM_LEFT		-1
#define PAGE_ANIM_NONE			0
#define PAGE_ANIM_FROM_RIGHT	1

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

	//indicator
	lv_obj_t * indicator;

	lv_style_t menu_style;

	//animation
	uint8_t state;
	lv_anim_t anim;
	uint32_t timer;

	//pages
	page_layout_t * page;
	page_layout_t * page_old;

	uint8_t pages_cnt;
	uint8_t actual_page;
);

void pages_load(char * filename, int8_t anim);

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
	lv_anim_set_exec_cb(&local->anim, pages_menu_anim_cb);
	lv_anim_set_values(&local->anim, 0, MENU_WIDTH);
	lv_anim_start(&local->anim);
	local->state = MENU_IN;
}

void pages_menu_hide()
{
	lv_anim_set_exec_cb(&local->anim, pages_menu_anim_cb);
	lv_anim_set_values(&local->anim, MENU_WIDTH, 0);
	lv_anim_start(&local->anim);
	local->state = MENU_OUT;
}

void pages_splash_show()
{
	lv_anim_set_exec_cb(&local->anim, pages_splash_anim_cb);

	lv_anim_set_values(&local->anim, lv_obj_get_height(local->mask) / 2, 0);
	local->mask_param = NULL;

	lv_anim_start(&local->anim);
	local->state = SPLASH_IN;
}

void pages_splash_hide()
{
	lv_anim_set_time(&local->anim, 1500);
	lv_anim_set_exec_cb(&local->anim, pages_splash_anim_cb);

	lv_anim_set_values(&local->anim, 0, lv_obj_get_height(local->mask) / 2);
	local->mask_param = NULL;

	lv_anim_start(&local->anim);
	local->state = SPLASH_OUT;
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
			widgets_unload(local->page_old);
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

	local->butt_short1 = lv_label_create(local->left_menu, NULL);
	lv_label_set_text(local->butt_short1, LV_SYMBOL_WIFI);
	lv_obj_align_origo(local->butt_short1, NULL, LV_ALIGN_CENTER, MENU_RADIUS / 2, MENU_HEIGHT / 5);


	local->right_menu = lv_cont_create(base, NULL);
	lv_obj_add_style(local->right_menu, LV_CONT_PART_MAIN, &local->menu_style);
	lv_obj_set_pos(local->right_menu, LV_HOR_RES, LV_VER_RES - MENU_HEIGHT - GUI_STATUSBAR_HEIGHT);
	lv_obj_set_size(local->right_menu, MENU_WIDTH + MENU_RADIUS, MENU_HEIGHT + MENU_RADIUS);

	local->butt_settings = lv_label_create(local->right_menu, NULL);
	lv_label_set_text(local->butt_settings, LV_SYMBOL_SETTINGS);
	lv_obj_align_origo(local->butt_settings, NULL, LV_ALIGN_IN_TOP_MID, -MENU_RADIUS / 2, MENU_HEIGHT / 4);

	local->butt_short2 = lv_label_create(local->right_menu, NULL);
	lv_label_set_text(local->butt_short2, LV_SYMBOL_BLUETOOTH);
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

	lv_obj_set_size(local->indicator, LV_HOR_RES, GUI_INDICATOR_HEIGHT);
	lv_obj_set_pos(local->indicator, 0, GUI_INDICATOR_Y_POS);
	lv_cont_set_layout(local->indicator, LV_LAYOUT_PRETTY_MID);

	for (uint8_t i = 0; i < local->pages_cnt; i++)
	{
		lv_obj_t * dot = lv_led_create(local->indicator, NULL);
		lv_obj_set_size(dot, GUI_INDICATOR_DOT_SIZE, GUI_INDICATOR_DOT_SIZE);
		lv_led_off(dot);
		if (i == local->actual_page)
			lv_led_on(dot);
	}

	local->state = MENU_IDLE;
}


static void pages_event_cb(lv_obj_t * obj, lv_event_t event)
{
    switch(event)
    {
        case LV_EVENT_LONG_PRESSED:
        	if (local->state == MENU_IDLE)
        		pages_menu_show();
            break;

        case LV_EVENT_SHORT_CLICKED:
        	//edit mode
        	if (local->state == MENU_IDLE)

		break;

        case LV_EVENT_KEY:
        {
        	uint32_t key = *((uint32_t *) lv_event_get_data());
        	if (local->state == MENU_SHOW)
        	{
        		switch (key)
        		{
    			case(LV_KEY_RIGHT):
    			case(LV_KEY_HOME):
    				gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    			break;
    			case(LV_KEY_LEFT):
    				gui_switch_task(&gui_page_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
//    				page_settings_open_page(name);
    			break;
    			case(LV_KEY_ENTER):
    				pages_power_off();
    			break;

        		}
        		return;
        	}
        	if (local->state == MENU_IDLE)
        	{
        		switch (key)
        		{
    			case(LV_KEY_RIGHT):
    				local->actual_page = (local->actual_page + 1) % local->pages_cnt;
					pages_load(pages_get_name(local->actual_page), PAGE_ANIM_FROM_RIGHT);

    			break;
    			case(LV_KEY_LEFT):
   					local->actual_page = ((local->actual_page > 0) ? local->actual_page : local->pages_cnt) - 1;

					pages_load(pages_get_name(local->actual_page), PAGE_ANIM_FROM_LEFT);
    			break;
    			case(LV_KEY_ENTER):

    			break;

        		}
        		return;
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

	int16_t w = lv_obj_get_width(local->mask);
	int16_t h = lv_obj_get_height(local->mask);

	//create new base layer for widgets
	local->page = (page_layout_t *) malloc(sizeof(page_layout_t));
	local->page->base = lv_obj_create(local->mask, NULL);
	lv_obj_set_style_local_bg_color(local->page->base, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
	lv_obj_set_size(local->page->base, w, h);
	lv_obj_set_pos(local->page->base, w * anim, 0);

	if (anim != PAGE_ANIM_NONE)
	{
		//create anim
		lv_anim_set_values(&local->anim, w * anim, 0);
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
			widgets_unload(local->page_old);
			lv_obj_del(local->page_old->base);
			free(local->page_old);
			local->page_old = NULL;
		}
	}

	pages_indicator_show();

	if (widgets_load_from_file(local->page, filename) == false)
	{
		//load default layout
//		lv_obj_t * cont = lv_cont_create(local->page->base, NULL);

		lv_obj_t * label = lv_label_create(local->page->base, NULL);
		lv_label_set_text_fmt(label, "Page '%s'\nnot found.", filename);
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
		lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);

		local->page->number_of_widgets = 0;
	}
	lv_obj_move_background(local->page->base);
}

static lv_obj_t * pages_init(lv_obj_t * par)
{
	lv_obj_set_style_local_bg_color(par, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_WHITE);

	lv_group_add_obj(gui.input.keypad, par);
	lv_obj_set_event_cb(par, pages_event_cb);

	local->mask = lv_objmask_create(par, NULL);
	lv_obj_set_pos(local->mask, 0, 0);
	lv_obj_set_size(local->mask, lv_obj_get_width(par), lv_obj_get_height(par));

	local->page = NULL;
	local->page_old = NULL;

	local->pages_cnt = pages_get_count();
	local->actual_page = config_get_int(&config.ui.page_last);

	pages_create_menu(local->mask);

	pages_load(pages_get_name(local->actual_page), PAGE_ANIM_NONE);

	local->state = MENU_IDLE;

	return par;
}

static void pages_loop()
{
	if (local->state == MENU_SHOW && local->timer < HAL_GetTick())
	{
		pages_menu_hide();
	}
}

static void pages_stop()
{
	lv_style_reset(&local->menu_style);
	widgets_unload(local->page);
	free(local->page);
}
