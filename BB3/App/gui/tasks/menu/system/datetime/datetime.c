
/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */
#define DEBUG_LEVEL DBG_DEBUG

#include "datetime.h"

#include <gui/tasks/menu/system/system.h>
#include <gui/tasks/menu/system/datetime/timeset.h>
#include <gui/tasks/menu/system/datetime/dateset.h>

#include "gui/gui_list.h"

#include "drivers/rtc.h"
#include "etc/format.h"
#include "etc/timezone.h"

#include "fc/fc.h"

REGISTER_TASK_IL(datetime,
    lv_obj_t * time;
    lv_obj_t * date;
    lv_obj_t * dst;
    lv_obj_t * tz;
    lv_obj_t * gnss;
);

static bool datetime_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
        if (obj == local->dst)
        {
        	//get utc time
        	uint64_t ts = fc_get_utc_time();

        	bool val = gui_list_switch_get_value(local->dst);
            config_set_bool(&config.time.dst, val);

            //set as utc
        	fc_set_time_from_utc(ts);
        }

        if (obj == local->tz)
        {
        	//get utc time
        	uint64_t ts = fc_get_utc_time();

        	//change timezone
            uint16_t val = gui_list_dropdown_get_value(local->tz);
            config_set_select(&config.time.zone, val);

            //set as utc
        	fc_set_time_from_utc(ts);
        }

        if (obj == local->gnss)
        {
            bool val = gui_list_switch_get_value(local->gnss);
            config_set_bool(&config.time.sync_gnss, val);
        }

	}

	if (event == LV_EVENT_CLICKED)
	{
        if (obj == local->time)
            gui_switch_task(&gui_timeset, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (obj == local->date)
            gui_switch_task(&gui_dateset, LV_SCR_LOAD_ANIM_MOVE_LEFT);

	}

	return true;
}


void datetime_loop()
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    rtc_get_time(&hour, &minute, &second);

    uint8_t day;
    uint8_t month;
    uint16_t year;

    rtc_get_date(&day, NULL, &month, &year);

    char buf[16];
    format_time(buf, hour, minute);
    gui_list_info_set_value(local->time, buf);

    format_date(buf, day, month, year);
    gui_list_info_set_value(local->date, buf);

}

lv_obj_t * datetime_init(lv_obj_t * par)
{
	DBG("settings init");

	lv_obj_t * list = gui_list_create(par, "Time & date", &gui_system, datetime_cb);

	uint8_t hour;
	uint8_t minute;
	uint8_t second;

    rtc_get_time(&hour, &minute, &second);

    uint8_t day;
    uint8_t month;
    uint16_t year;

    rtc_get_date(&day, NULL, &month, &year);

	char buf[16];
	format_time(buf, hour, minute);
	local->time = gui_list_info_add_entry(list, "Time", buf);

    format_date(buf, day, month, year);
	local->date = gui_list_info_add_entry(list, "Date", buf);

	local->dst = gui_list_switch_add_entry(list, "Daylight saving time", config_get_bool(&config.time.dst));

    #define OPTION_LEN  10 //UTC+00:11,
	char options[NUMBER_OF_TIMEZONES * OPTION_LEN + 1];
	for (uint8_t i = 0; i < NUMBER_OF_TIMEZONES; i++)
	{
	    int32_t offset = timezone_get_offset(i, false);
	    char c = (offset >= 0) ? '+' : '-';
	    offset = abs(offset);
	    uint8_t h = offset / 3600;
	    uint8_t m = (offset % 3600) / 60;

	    sprintf(options + (i * OPTION_LEN), "UTC%c%02u:%02u\n", c, h, m);
	}
	options[NUMBER_OF_TIMEZONES * OPTION_LEN - 1] = 0;

	local->tz = gui_list_dropdown_add_entry(list, "Time zone", options, config_get_select(&config.time.zone));

	local->gnss = gui_list_switch_add_entry(list, "Sync with GNSS", config_get_bool(&config.time.sync_gnss));

	return list;
}
