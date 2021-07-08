/*
 * statusbar.c
 *
 *  Created on: Aug 17, 2020
 *      Author: horinek
 */

#include "statusbar.h"
#include "drivers/rtc.h"
#include "drivers/power/pwr_mng.h"

void statusbar_show()
{
	//animation
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, gui.statusbar.bar);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);
    lv_anim_set_values(&a, -GUI_STATUSBAR_HEIGHT, 0);
	lv_anim_start(&a);
}

void statusbar_hide()
{
	//animation
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, gui.statusbar.bar);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);
    lv_anim_set_values(&a, 0, -GUI_STATUSBAR_HEIGHT);
	lv_anim_start(&a);
}

void statusbar_create()
{
	lv_obj_t * bg = lv_obj_create(lv_layer_sys(), NULL);
	lv_obj_set_size(bg, LV_HOR_RES, GUI_STATUSBAR_HEIGHT);
	lv_obj_set_style_local_bg_color(bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

	gui.statusbar.bar = lv_obj_create(bg, NULL);
	lv_obj_set_size(gui.statusbar.bar, LV_HOR_RES, GUI_STATUSBAR_HEIGHT);

	gui.statusbar.time = lv_label_create(gui.statusbar.bar, NULL);
	lv_obj_align(gui.statusbar.time, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);

    gui.statusbar.icons = lv_label_create(gui.statusbar.bar, NULL);
    lv_obj_align(gui.statusbar.icons, NULL, LV_ALIGN_IN_RIGHT_MID, -5, 0);

    gui.statusbar.gray_icons = lv_label_create(gui.statusbar.bar, NULL);
    lv_obj_align(gui.statusbar.gray_icons, gui.statusbar.icons, LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_obj_set_style_local_text_color(gui.statusbar.gray_icons, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);


	statusbar_step();
	statusbar_show();

	//message BOX
	gui.statusbar.mbox = lv_cont_create(lv_layer_sys(), NULL);
	lv_obj_set_pos(gui.statusbar.mbox, 0, GUI_STATUSBAR_HEIGHT);
	lv_obj_set_size(gui.statusbar.mbox, LV_HOR_RES, 0);
	lv_obj_set_style_local_bg_opa(gui.statusbar.mbox, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_pad_all(gui.statusbar.mbox, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_cont_set_fit2(gui.statusbar.mbox, LV_FIT_NONE, LV_FIT_TIGHT);
	lv_cont_set_layout(gui.statusbar.mbox, LV_LAYOUT_COLUMN_LEFT);

//	statusbar_add_msg(STATUSBAR_MSG_ERROR, "GNSS Error");
//	statusbar_add_msg(STATUSBAR_MSG_WARN, "Battery low");
//	statusbar_add_msg(STATUSBAR_MSG_INFO, "Uploading tracklog...\n\n");
}

void statusbar_msg_anim_hide_cb(lv_anim_t * a)
{
	lv_obj_del(a->var);
}

void statusbar_msg_anim_show_cb(lv_anim_t * a)
{
	lv_anim_t new_a;
	lv_anim_init(&new_a);
	lv_anim_set_var(&new_a, a->var);
    lv_anim_set_exec_cb(&new_a, (lv_anim_exec_xcb_t) lv_obj_set_height);
    lv_anim_set_values(&new_a, lv_obj_get_height(a->var), 0);
    lv_anim_set_ready_cb(&new_a, statusbar_msg_anim_hide_cb);
    lv_anim_set_delay(&new_a, GUI_MSG_TIMEOUT);
	lv_anim_start(&new_a);
}

void statusbar_add_msg(statusbar_msg_type_t type, char * text)
{
    gui_lock_acquire();

	lv_color_t colors[] = {LV_COLOR_GREEN, LV_COLOR_YELLOW, LV_COLOR_RED};

	lv_obj_t * msg = lv_cont_create(gui.statusbar.mbox, NULL);
	lv_cont_set_fit2(msg, LV_FIT_NONE, LV_FIT_TIGHT);
	lv_obj_set_size(msg, LV_HOR_RES, 0);
	lv_cont_set_layout(msg, LV_LAYOUT_CENTER);

	lv_obj_set_style_local_bg_opa(msg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
	lv_obj_set_style_local_bg_color(msg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, colors[type]);

	lv_obj_t * label = lv_label_create(msg, NULL);
	lv_label_set_text(label, text);

	lv_cont_set_fit(msg, LV_FIT_NONE);

	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, msg);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_height);
    lv_anim_set_values(&a, 0, lv_obj_get_height(msg));
    lv_anim_set_ready_cb(&a, statusbar_msg_anim_show_cb);
	lv_anim_start(&a);

	gui_lock_release();
}

void statusbar_step()
{
	uint8_t h;
	uint8_t m;
	uint8_t s;

	if (rtc_is_valid())
	{
		rtc_get_time(&h, &m, &s);
		lv_label_set_text_fmt(gui.statusbar.time, "%02u:%02u", h, m);
	}
	else
	{
		if (BLINK(2000))
		{
			lv_label_set_text(gui.statusbar.time, "--:--");
		}
		else
		{
			if (config_get_bool(&config.time.sync_gnss))
				lv_label_set_text(gui.statusbar.time, "No GNSS");
			else
				lv_label_set_text(gui.statusbar.time, "Set time");
		}
	}

    char icons[64];
    char gray_icons[64];

    strcpy(icons, "");
    strcpy(gray_icons, "");

    if (fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_auto)
    {
        sprintf(icons + strlen(icons), " " LV_SYMBOL_DOWNLOAD);
    }
    else
    {
        // if there is bt connection active
        if (fc.esp.state & ESP_STATE_BT_ON)
        {
            if (fc.esp.state & ESP_STATE_BT_DATA || fc.esp.state & ESP_STATE_BT_AUDIO)
                sprintf(icons + strlen(icons), " " LV_SYMBOL_BLUETOOTH);
            else
                sprintf(gray_icons + strlen(gray_icons), " " LV_SYMBOL_BLUETOOTH);
        }

        //if it is connected to the wifi
        if (fc.esp.state & ESP_STATE_WIFI_CLIENT)
        {
            if (fc.esp.state & ESP_STATE_WIFI_CONNECTED)
                sprintf(icons + strlen(icons), " " LV_SYMBOL_WIFI);
            else
                sprintf(gray_icons + strlen(gray_icons), " " LV_SYMBOL_WIFI);
        }

        //if it is connected to the wifi
        if (fc.esp.state & ESP_STATE_WIFI_AP)
        {
            if (fc.esp.state & ESP_STATE_WIFI_AP_CONNECTED)
                sprintf(icons + strlen(icons), " AP ");
            else
                sprintf(gray_icons + strlen(gray_icons), " AP ");
        }
    }

    if (pwr.data_port != PWR_DATA_NONE)
    {
        sprintf(icons + strlen(icons), " " LV_SYMBOL_USB);
    }

	if (pwr.charger.charge_port != PWR_CHARGE_NONE)
	{
	    sprintf(icons + strlen(icons), " " LV_SYMBOL_CHARGE);
	}

	sprintf(icons + strlen(icons), " %u%%", pwr.fuel_gauge.battery_percentage);

	char bat_icon[4];
    if (pwr.fuel_gauge.battery_percentage < 20)
        strcpy(bat_icon, LV_SYMBOL_BATTERY_EMPTY);
    else if (pwr.fuel_gauge.battery_percentage < 40)
        strcpy(bat_icon, LV_SYMBOL_BATTERY_1);
    else if (pwr.fuel_gauge.battery_percentage < 60)
        strcpy(bat_icon, LV_SYMBOL_BATTERY_2);
    else if (pwr.fuel_gauge.battery_percentage < 80)
        strcpy(bat_icon, LV_SYMBOL_BATTERY_3);
    else
        strcpy(bat_icon, LV_SYMBOL_BATTERY_FULL);

    sprintf(icons + strlen(icons), " %s", bat_icon);

    lv_label_set_text(gui.statusbar.icons, icons);
    lv_label_set_text(gui.statusbar.gray_icons, gray_icons);

    lv_obj_realign(gui.statusbar.icons);
    lv_obj_realign(gui.statusbar.gray_icons);
}
