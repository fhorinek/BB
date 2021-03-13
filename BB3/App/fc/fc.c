/*
 * fc.cc
 *
 *  Created on: May 6, 2020
 *      Author: horinek
 */


#include "fc.h"

#include "etc/epoch.h"
#include "etc/timezone.h"

#include "config/config.h"

#include "drivers/rtc.h"

fc_t fc __attribute__ ((aligned (4)));
osSemaphoreId_t lock_fc_global;

void fc_init()
{
    //create released semaphore
    lock_fc_global = osSemaphoreNew(1, 1, NULL);
    vQueueAddToRegistry(lock_fc_global, "lock_fc_global");

	INFO("Flight computer init");
}

void fc_device_status(char * buff, fc_device_status_t status)
{
    switch(status)
    {
        case(fc_dev_init):
            strcpy(buff, "Device init");
        break;

        case(fc_dev_ready):
            strcpy(buff, "Device ready");
        break;

        case(fc_dev_error):
            strcpy(buff, "Device error");
        break;

        case(fc_device_not_calibrated):
            strcpy(buff, "Not calibrated");
        break;

        case(fc_dev_off):
            strcpy(buff, "Device disabled");
        break;
    }
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

float fc_alt_to_qnh(float alt, float pressure)
{
    return pressure / pow(1.0 - (alt / 44330.0), 5.255);
}

float fc_press_to_alt(float pressure, float qnh)
{
    return 44330.0 * (1 - pow((pressure / qnh), 0.190295));
}

float fc_alt_to_press(float alt, float qnh)
{
    return qnh * pow(1.0 - (alt / 44330.0), 5.255);
}

