/*
 * statusbar.c
 *
 *  Created on: Aug 17, 2020
 *      Author: horinek
 */

#include "statusbar.h"
#include "drivers/rtc.h"
#include "drivers/power/pwr_mng.h"

#include "etc/format.h"
#include "fc/logger/logger.h"

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

#define 	I_MASK		0b00111111
#define 	I_HIDE		0
#define 	I_SHOW		1
#define 	I_GRAY		2
#define 	I_YELLOW	3
#define 	I_RED		4

#define 	I_BLINK		0b10000000
#define 	I_FAST		0b01000000


static void set_icon(uint8_t index, uint8_t state)
{
	lv_obj_t * icon = gui.statusbar.icons[index];

	uint16_t blink_dura = (state & I_FAST) ? 500 : 2000;

	if (state & I_BLINK && BLINK(blink_dura))
	{
		lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_obj_set_hidden(icon, false);
		lv_label_set_long_mode(icon, LV_LABEL_LONG_EXPAND);
	}
	else
	{
		switch(state & I_MASK)
		{
			case(I_HIDE):
				lv_obj_set_hidden(icon, true);
				lv_label_set_long_mode(icon, LV_LABEL_LONG_CROP);
				lv_obj_set_width(icon, 0);
				return;
			break;
			case(I_SHOW):
				lv_obj_set_hidden(icon, false);
				lv_label_set_long_mode(icon, LV_LABEL_LONG_EXPAND);
				lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
			break;

			case(I_GRAY):
				lv_obj_set_hidden(icon, false);
				lv_label_set_long_mode(icon, LV_LABEL_LONG_EXPAND);
				lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
			break;
			case(I_YELLOW):
				lv_obj_set_hidden(icon, false);
				lv_label_set_long_mode(icon, LV_LABEL_LONG_EXPAND);
				lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
			break;
			case(I_RED):
				lv_obj_set_hidden(icon, false);
				lv_label_set_long_mode(icon, LV_LABEL_LONG_EXPAND);
				lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
			break;
		}
	}
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

	for (uint8_t i = 0; i < BAR_ICON_CNT; i++)
	{
		gui.statusbar.icons[i] = lv_label_create(gui.statusbar.bar, NULL);
		if (i == 0)
			lv_obj_align(gui.statusbar.icons[i], NULL, LV_ALIGN_IN_RIGHT_MID, -5, 0);
		else
			lv_obj_align(gui.statusbar.icons[i], gui.statusbar.icons[i - 1], LV_ALIGN_OUT_LEFT_MID, 0, 0);

		set_icon(i, I_HIDE);
	}

	lv_label_set_text(gui.statusbar.icons[BAR_ICON_BAT], LV_SYMBOL_BATTERY_EMPTY);
	lv_label_set_text(gui.statusbar.icons[BAR_ICON_CHARGE], LV_SYMBOL_CHARGE " ");
	lv_label_set_text(gui.statusbar.icons[BAR_ICON_USB], LV_SYMBOL_USB " ");
	lv_label_set_text(gui.statusbar.icons[BAR_ICON_GNSS], LV_SYMBOL_GPS " ");
	lv_label_set_text(gui.statusbar.icons[BAR_ICON_LOG], LV_SYMBOL_FILE " ");
	lv_label_set_text(gui.statusbar.icons[BAR_ICON_BT], LV_SYMBOL_BLUETOOTH " ");
	lv_label_set_text(gui.statusbar.icons[BAR_ICON_AP], "AP ");
	lv_label_set_text(gui.statusbar.icons[BAR_ICON_WIFI], LV_SYMBOL_WIFI " ");
	lv_label_set_text(gui.statusbar.icons[BAR_ICON_SYS], LV_SYMBOL_SETTINGS " ");
	lv_label_set_text(gui.statusbar.icons[BAR_ICON_DL], LV_SYMBOL_DOWNLOAD " ");

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
	lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_width(label, LV_HOR_RES - 10);
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

	if (rtc_is_valid())
	{
		uint8_t h;
		uint8_t m;
		uint8_t s;

		rtc_get_time(&h, &m, &s);
		char buff[8];
		format_time(buff, h, m);
		lv_label_set_text(gui.statusbar.time, buff);
	}
	else
	{
		if (BLINK(2000))
		{
			if (config_get_bool(&config.time.sync_gnss))
				lv_label_set_text(gui.statusbar.time, "No GNSS");
			else
				lv_label_set_text(gui.statusbar.time, "Set time");
		}
		else
		{
			lv_label_set_text(gui.statusbar.time, "--:--");
		}
	}

    if (fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual)
    {
    	set_icon(BAR_ICON_SYS, I_SHOW | I_FAST | I_BLINK);
    	set_icon(BAR_ICON_WIFI, I_HIDE);
    	set_icon(BAR_ICON_AP, I_HIDE);
    	set_icon(BAR_ICON_BT, I_HIDE);
    }
    else
    {
    	set_icon(BAR_ICON_SYS, I_HIDE);
        // if there is bt connection active
        if (fc.esp.state & ESP_STATE_BT_ON)
        {
            if (fc.esp.state & ESP_STATE_BT_A2DP || fc.esp.state & ESP_STATE_BT_SPP || fc.esp.state & ESP_STATE_BT_BLE)
            	set_icon(BAR_ICON_BT, I_SHOW);
            else
            	set_icon(BAR_ICON_BT, I_GRAY);
        }
        else
        {
        	set_icon(BAR_ICON_BT, I_HIDE);
        }

        //if it is connected to the wifi
        if (fc.esp.state & ESP_STATE_WIFI_CLIENT)
        {
            if (fc.esp.state & ESP_STATE_WIFI_CONNECTED)
            	set_icon(BAR_ICON_WIFI, I_SHOW);
            else
            	set_icon(BAR_ICON_WIFI, I_GRAY);
        }
        else
        {
        	set_icon(BAR_ICON_WIFI, I_HIDE);
        }

        //if something is connecected to AP
        if (fc.esp.state & ESP_STATE_WIFI_AP)
        {
            if (fc.esp.state & ESP_STATE_WIFI_AP_CONNECTED)
            	set_icon(BAR_ICON_AP, I_SHOW);
            else
            	set_icon(BAR_ICON_AP, I_GRAY);
        }
        else
        {
        	set_icon(BAR_ICON_AP, I_HIDE);
        }

    }

    if (fc.gnss.fix == 3)
    {
    	set_icon(BAR_ICON_GNSS, I_SHOW);
    }
    else if (fc.gnss.fix == 2)
    {
    	set_icon(BAR_ICON_GNSS, I_YELLOW);
    }
    else
    {
    	set_icon(BAR_ICON_GNSS, I_SHOW | I_BLINK);
    }

    fc_logger_status_t logger = logger_state();
    if (logger == fc_logger_record)
    {
    	set_icon(BAR_ICON_LOG, I_SHOW);
    }
    else if (logger == fc_logger_wait)
    {
    	set_icon(BAR_ICON_LOG, I_SHOW | I_BLINK);
    }
    else
    {
    	set_icon(BAR_ICON_LOG, I_HIDE);
    }


	set_icon(BAR_ICON_USB, (pwr.data_port != PWR_DATA_NONE) ? I_SHOW : I_HIDE);
	set_icon(BAR_ICON_CHARGE, (pwr.charger.charge_port > PWR_CHARGE_WEAK) ? I_SHOW : I_HIDE);
	if (pwr.charger.charge_port == PWR_CHARGE_WEAK)
	{
		set_icon(BAR_ICON_CHARGE, I_RED | I_FAST | I_BLINK);
	}


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

    if (pwr.fuel_gauge.battery_percentage > 10)
    	set_icon(BAR_ICON_BAT, I_SHOW);
    else if (pwr.fuel_gauge.battery_percentage > 5)
		set_icon(BAR_ICON_BAT, I_YELLOW);
    else
    	set_icon(BAR_ICON_BAT, I_RED);

    if (config_get_bool(&config.display.bat_per) && pwr.fuel_gauge.status == fc_dev_ready)
    {
    	lv_label_set_text_fmt(gui.statusbar.icons[BAR_ICON_BAT], "%u%%%s", pwr.fuel_gauge.battery_percentage, bat_icon);
    }
    else
    {
    	lv_label_set_text(gui.statusbar.icons[BAR_ICON_BAT], bat_icon);
    }

	for (uint8_t i = 0; i < BAR_ICON_CNT; i++)
	{
		lv_obj_realign(gui.statusbar.icons[i]);
	}
}
