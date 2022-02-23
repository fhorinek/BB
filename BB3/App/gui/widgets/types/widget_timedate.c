/*
 * widget_timedate.c
 *
 *  Created on: 21. 1. 2022
 *      Author: tilmann@bubecks.de
 *
 * Show current time and date taken from RTC.
 */
#include "gui/widgets/widget.h"
#include "drivers/power/pwr_mng.h"
#include "drivers/rtc.h"

REGISTER_WIDGET_IU
(
    TimeDate,
    "Time - time and date",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_alt_unit),

    lv_obj_t * value;
);


static void TimeDate_init(lv_obj_t * base, widget_slot_t * slot)
{
	char *title;

	widget_create_base(base, slot);

    if (!widget_flag_is_set(slot, wf_label_hide))
    {
        if (widget_flag_is_set(slot, wf_alt_unit))
        	title = "Time/Date";
        else
        	title = "Time";
    	widget_add_title(base, slot, title);
    }

    local->value = widget_add_value(base, slot, NULL, NULL);
}

static void TimeDate_update(widget_slot_t * slot)
{
	if (rtc_is_valid())
	{
		uint8_t h;
		uint8_t m;
		uint8_t s;
		char value[20];

		rtc_get_time(&h, &m, &s);
		format_time(value, h, m);
        if (widget_flag_is_set(slot, wf_alt_unit))
        {
        	char *end;
        	uint8_t day, wday, month;
        	uint16_t year;

    		rtc_get_date(&day, &wday, &month, &year);
        	end = value + strlen(value);
        	*end = '\n'; end++;
        	format_date(end, day, month, year);
        }
		lv_label_set_text(local->value, value);
		widget_update_font_size(local->value);
	}
}


