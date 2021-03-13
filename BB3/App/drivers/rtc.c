/*
 * rtc.c
 *
 *  Created on: Sep 25, 2020
 *      Author: horinek
 */

#include "rtc.h"

#define RTC_TAMP_SET    0xDEADBEEF
#define RTC_TAMP_GPS    0xC0FFEE

#define RTC_START_YEAR  2020

bool rtc_is_set()
{
    if (TAMP->BKP0R != RTC_TAMP_SET)
        if (TAMP->BKP0R != RTC_TAMP_GPS)
            return false;

    return true;
}

void rtc_init()
{
    MX_RTC_Init();

    if (TAMP->BKP0R != RTC_TAMP_SET)
    {
        rtc_set_time(12, 00, 00);
        rtc_set_date(1 , 2, 1, 2020);

        TAMP->BKP0R = RTC_TAMP_SET;
    }
}

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
	sDate.Year = year - RTC_START_YEAR;

	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void rtc_get_date(uint8_t * day, uint8_t * wday, uint8_t * month, uint16_t * year)
{
	RTC_DateTypeDef sDate = {0};

	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	*day = sDate.Date;
	*month = sDate.Month;
	*year = sDate.Year + RTC_START_YEAR;

	if (wday != NULL)
	    *wday = sDate.Month;
}

uint64_t rtc_get_epoch()
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    return datetime_to_epoch(sTime.Seconds, sTime.Minutes, sTime.Hours, sDate.Date, sDate.Month, sDate.Year + RTC_START_YEAR);
}

