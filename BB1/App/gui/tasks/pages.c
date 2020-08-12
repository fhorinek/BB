/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "pages.h"

#include "settings.h"

#include "../gui_list.h"
#include "../map/tile.h"

#include "../widgets/widgets.h"

extern const lv_img_dsc_t tile;

#define MENU_TIMEOUT	20000

#define MENU_RADIUS	15
#define MENU_WIDTH	35
#define MENU_HEIGHT	180

#define MENU_IDLE	0
#define MENU_IN		1
#define MENU_SHOW	2
#define MENU_OUT	3


typedef struct
{
	lv_obj_t * left_menu;
	lv_obj_t * right_menu;
	lv_obj_t * center_menu;

	lv_obj_t * butt_layout;
	lv_obj_t * butt_short1;
	lv_obj_t * butt_power;
	lv_obj_t * butt_short2;
	lv_obj_t * butt_settings;

	lv_style_t menu_style;

	uint8_t menu_state;
	lv_anim_t menu_anim;
	uint32_t menu_timer;

	page_layout_t page;
} local_vars_t;


void pages_menu_show()
{
	lv_anim_set_values(&local.menu_anim, 0, MENU_WIDTH);
	lv_anim_start(&local.menu_anim);
	local.menu_state = MENU_IN;
}

void pages_menu_hide()
{
	lv_anim_set_values(&local.menu_anim, MENU_WIDTH, 0);
	lv_anim_start(&local.menu_anim);
	local.menu_state = MENU_OUT;
}

void pages_menu_anim_cb(void * obj, lv_anim_value_t val)
{
	lv_style_set_bg_opa(&local.menu_style, LV_STATE_DEFAULT, val * 5);

	lv_obj_set_x(local.left_menu, val - MENU_RADIUS - MENU_WIDTH);
	lv_obj_set_x(local.right_menu, LV_HOR_RES - val);

	lv_obj_set_width(local.center_menu, LV_HOR_RES - val * 2);
	lv_obj_set_pos(local.center_menu, val, LV_VER_RES - val);
}

void pages_menu_anim_ready_cb(lv_anim_t * a)
{
	(void)a;

	if (local.menu_state == MENU_IN)
	{
		local.menu_state = MENU_SHOW;
		local.menu_timer = HAL_GetTick() + MENU_TIMEOUT;
	}

	if (local.menu_state == MENU_OUT)
	{
		local.menu_state = MENU_IDLE;
	}

}


void pages_create_menu(lv_obj_t * base)
{
	lv_style_init(&local.menu_style);
	lv_style_set_bg_color(&local.menu_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_border_width(&local.menu_style, LV_STATE_DEFAULT, 0);
	lv_style_set_radius(&local.menu_style, LV_STATE_DEFAULT, MENU_RADIUS);


	local.left_menu = lv_cont_create(base, NULL);
	lv_obj_add_style(local.left_menu, LV_CONT_PART_MAIN, &local.menu_style);
	lv_obj_set_pos(local.left_menu, -MENU_RADIUS - MENU_WIDTH, LV_VER_RES - MENU_HEIGHT);
	lv_obj_set_size(local.left_menu, MENU_WIDTH + MENU_RADIUS, MENU_HEIGHT + MENU_RADIUS);

	local.butt_layout = lv_label_create(local.left_menu, NULL);
	lv_label_set_text(local.butt_layout, LV_SYMBOL_LIST);
	lv_obj_align_origo(local.butt_layout, NULL, LV_ALIGN_IN_TOP_MID, MENU_RADIUS / 2, MENU_HEIGHT / 4);

	local.butt_short1 = lv_label_create(local.left_menu, NULL);
	lv_label_set_text(local.butt_short1, LV_SYMBOL_WIFI);
	lv_obj_align_origo(local.butt_short1, NULL, LV_ALIGN_CENTER, MENU_RADIUS / 2, MENU_HEIGHT / 5);


	local.right_menu = lv_cont_create(base, NULL);
	lv_obj_add_style(local.right_menu, LV_CONT_PART_MAIN, &local.menu_style);
	lv_obj_set_pos(local.right_menu, LV_HOR_RES, LV_VER_RES - MENU_HEIGHT);
	lv_obj_set_size(local.right_menu, MENU_WIDTH + MENU_RADIUS, MENU_HEIGHT + MENU_RADIUS);

	local.butt_settings = lv_label_create(local.right_menu, NULL);
	lv_label_set_text(local.butt_settings, LV_SYMBOL_SETTINGS);
	lv_obj_align_origo(local.butt_settings, NULL, LV_ALIGN_IN_TOP_MID, -MENU_RADIUS / 2, MENU_HEIGHT / 4);

	local.butt_short2 = lv_label_create(local.right_menu, NULL);
	lv_label_set_text(local.butt_short2, LV_SYMBOL_BLUETOOTH);
	lv_obj_align_origo(local.butt_short2, NULL, LV_ALIGN_CENTER, -MENU_RADIUS / 2, MENU_HEIGHT / 5);


	local.center_menu = lv_cont_create(base, NULL);
	lv_obj_add_style(local.center_menu, LV_CONT_PART_MAIN, &local.menu_style);
	lv_obj_set_pos(local.center_menu, 0, LV_VER_RES);
	lv_obj_set_size(local.center_menu, LV_HOR_RES, MENU_WIDTH);
	lv_obj_set_style_local_radius(local.center_menu, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_cont_set_layout(local.center_menu, LV_LAYOUT_CENTER);

	local.butt_power = lv_label_create(local.center_menu, NULL);
	lv_label_set_text(local.butt_power, LV_SYMBOL_POWER);

	lv_anim_init(&local.menu_anim);
	lv_anim_set_exec_cb(&local.menu_anim, pages_menu_anim_cb);
	lv_anim_set_ready_cb(&local.menu_anim, pages_menu_anim_ready_cb);

	local.menu_state = MENU_IDLE;

	lv_obj_set_top(local.left_menu, true);
	lv_obj_move_foreground(local.right_menu);
	lv_obj_move_foreground(local.center_menu);

}

static void pages_event_cb(lv_obj_t * obj, lv_event_t event)
{
	INFO("Event: %u", event);

    switch(event)
    {
        case LV_EVENT_LONG_PRESSED:
        	if (local.menu_state == MENU_IDLE)
        		pages_menu_show();
        	if (local.menu_state == MENU_SHOW)
        		pages_menu_hide();
            break;
    }

}

static lv_obj_t * pages_init(lv_obj_t * par)
{
	lv_obj_t * base = lv_obj_create(par, NULL);
	lv_obj_set_size(base, LV_HOR_RES, LV_VER_RES);
	lv_obj_set_style_local_border_width(base, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, 0);

	lv_group_add_obj(gui_group, base);
	lv_obj_set_event_cb(base, pages_event_cb);

	widgets_load_from_file(par, &local.page, "test");

	pages_create_menu(base);

	return base;
}

static void pages_loop()
{
	if (local.menu_state == MENU_SHOW && local.menu_timer < HAL_GetTick())
	{
		pages_menu_hide();
	}
}

static bool pages_stop()
{
	lv_style_reset(&local.menu_style);
	widgets_unload(&local.page);

	return true;
}

gui_task_t gui_pages =
{
	pages_init,
	pages_loop,
	pages_stop
};
