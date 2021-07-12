/*
 * epoch.cc
 *
 *  Created on: May 8, 2020
 *      Author: horinek
 */

#include "epoch.h"

uint8_t monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#define LEAP_YEAR(_year) ((_year % 4)==0)

uint64_t datetime_to_epoch(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint16_t year)
{
    uint16_t i;
    uint64_t timestamp;

    // seconds from 1970 till 1 jan 00:00:00 this year
    timestamp = ((int64_t)year - 1970l) * (60l * 60l * 24l * 365l);

    // add extra days for leap years
    for (i = 1970; i < year; i++)
    {
        if (LEAP_YEAR(i))
        {
            timestamp += 60 * 60 * 24L;
        }
    }
    // add days for this year
    for (i = 0; i < (uint16_t) (month - 1); i++)
    {
        if (i == 1 && LEAP_YEAR(year))
        {
            timestamp += 60 * 60 * 24L * 29;
        }
        else
        {
            timestamp += 60 * 60 * 24L * monthDays[i];
        }
    }

    timestamp += (day - 1) * 3600 * 24L;
    timestamp += hour * 3600L;
    timestamp += min * 60L;
    timestamp += sec;
    return timestamp;
}

uint8_t datetime_number_of_days(uint8_t month, uint16_t year)
{
    if (LEAP_YEAR(year) && month == 2)
        return 29;

    return monthDays[month - 1];
}

uint8_t datetime_wday_from_epoch(uint64_t epoch)
{
    return ((epoch / (60 * 60 * 24)) + 4) % 7;
}

void time_from_epoch(uint64_t epoch, uint8_t * psec, uint8_t * pmin, uint8_t * phour)
{
	*psec=epoch%60;
	epoch/=60; // now it is minutes
	*pmin=epoch%60;
	epoch/=60; // now it is hours
	*phour=epoch%24;
}


void datetime_from_epoch(uint64_t epoch, uint8_t *psec, uint8_t *pmin, uint8_t *phour, uint8_t *pday, uint8_t *pwday, uint8_t *pmonth, uint16_t *pyear)
{
    uint8_t year;
    uint8_t month, monthLength;
    uint64_t days;

    *psec = epoch % 60;
    epoch /= 60; // now it is minutes
    *pmin = epoch % 60;
    epoch /= 60; // now it is hours
    *phour = epoch % 24;
    epoch /= 24; // now it is days

    *pwday = (epoch + 4) % 7;

    year = 70;
    days = 0;
    while ((unsigned) (days += (LEAP_YEAR(year) ? 366 : 365)) <= epoch)
    {
        year++;
    }
    *pyear = year + 1900; // *pyear is returned as years from 1900

    days -= LEAP_YEAR(year) ? 366 : 365;
    epoch -= days; // now it is days in this year, starting at 0

    days = 0;
    month = 0;
    monthLength = 0;
    for (month = 0; month < 12; month++)
    {
        if (month == 1)
        { // february
            if (LEAP_YEAR(year))
            {
                monthLength = 29;
            }
            else
            {
                monthLength = 28;
            }
        }
        else
        {
            monthLength = monthDays[month];
        }

        if (epoch >= monthLength)
        {
            epoch -= monthLength;
        }
        else
        {
            break;
        }
    }

    *pmonth = month + 1;  // jan is month 1
    *pday = epoch + 1;  // day of month
}

