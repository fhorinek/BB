/*
 * fc.cc
 *
 *  Created on: May 6, 2020
 *      Author: horinek
 */


#include "fc.h"

#include "../etc/epoch.h"
#include "../etc/timezone.h"

#include "../config/config.h"

#include "../drivers/rtc.h"

fc_t fc;

void fc_init()
{
	INFO("Flight computer init");
	//Release the semaphore
	osSemaphoreRelease(fc_global_lockHandle);
}

void fc_set_time(uint32_t datetime)
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint8_t wday;

	datetime_from_epoch(datetime, &second, &minute, &hour, &day, &wday, &month, &year);

	rtc_set_time(hour, minute, second);
	rtc_set_date(day, wday, month, year);
}

void fc_set_time_from_utc(uint32_t datetime)
{
	int32_t delta = timezone_get_offset(config_get_select(&config.settings.time.zone));
	fc_set_time(datetime + delta);
}
