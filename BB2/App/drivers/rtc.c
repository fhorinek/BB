/*
 * rtc.c
 *
 *  Created on: Sep 25, 2020
 *      Author: horinek
 */

#include "rtc.h"

void rtc_get_time(uint8_t * hour, uint8_t * minute, uint8_t * second)
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

	*hour = sTime.Hours;
	*minute = sTime.Minutes;
	*second = sTime.Seconds;

	//Need to get the date to unlock the shadow register
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void rtc_set_time(uint8_t hour, uint8_t minute, uint8_t second)
{
	RTC_TimeTypeDef sTime = {0};

	sTime.Hours = hour;
	sTime.Minutes = minute;
	sTime.Seconds = second;
	sTime.SubSeconds = 0;

	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
}

void rtc_set_date(uint8_t day, uint8_t wday, uint8_t month, uint16_t year)
{
	RTC_DateTypeDef sDate = {0};

	sDate.Date = day;
	sDate.WeekDay = wday;
	sDate.Month = month;
	sDate.Year = year;

	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void rtc_get_date(uint8_t * day, uint8_t * wday, uint8_t * month, uint16_t * year)
{
	RTC_DateTypeDef sDate = {0};

	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	*day = sDate.Date;
	*month = sDate.Month;
	*year = sDate.Year;
	*wday = sDate.Month;
}

