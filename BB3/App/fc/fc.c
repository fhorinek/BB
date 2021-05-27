/*
 * fc.cc
 *
 *  Created on: May 6, 2020
 *      Author: horinek
 */


#include "fc.h"

#include "etc/epoch.h"
#include "etc/timezone.h"

#include "drivers/rtc.h"

fc_t fc __attribute__ ((aligned (4)));
osSemaphoreId_t lock_fc_global;

void fc_reset()
{
    if (fc.fused.status != fc_dev_ready)
    {
        fc.flight.mode = flight_not_ready;
    }
    else
    {
        fc.flight.mode = flight_wait_to_takeoff;
        fc.autostart.altitude = fc.fused.altitude1;
        fc.autostart.timestamp = HAL_GetTick();
    }
}

void fc_init()
{
    //create released semaphore
    lock_fc_global = osSemaphoreNew(1, 1, NULL);
    vQueueAddToRegistry(lock_fc_global, "lock_fc_global");

	INFO("Flight computer init");

	vario_profile_load(config_get_text(&profile.vario));

	fc_reset();
}

void fc_takeoff()
{
    INFO("Take-off");
    fc.flight.start_alt = fc.fused.altitude1;
    fc.flight.start_time = HAL_GetTick();

    fc.autostart.timestamp = HAL_GetTick();
    fc.autostart.altitude = fc.fused.altitude1;

    fc.flight.mode = flight_flight;
}

void fc_landing()
{
    INFO("Landing");
    fc.flight.duration = (HAL_GetTick() - fc.flight.start_time) / 1000;
    fc.flight.mode = flight_landed;
}

void fc_step()
{
    if (fc.flight.mode == flight_wait_to_takeoff)
    {
        if (config_get_bool(&profile.flight.auto_take_off.alt_change_enabled))
        {
            if (abs(fc.autostart.altitude - fc.fused.altitude1) >
                config_get_int(&profile.flight.auto_take_off.alt_change_value))
            {
                fc_takeoff();
            }
        }

        if (config_get_bool(&profile.flight.auto_take_off.speed_enabled))
        {
            if (fc.gnss.fix == 3)
            {
                if (fc.gnss.ground_speed >
                    config_get_int(&profile.flight.auto_take_off.speed_value))
                {
                    fc_takeoff();
                }
            }
        }

        uint32_t delta = HAL_GetTick() - fc.autostart.timestamp;
        if (delta > config_get_int(&profile.flight.auto_take_off.timeout) * 1000)
        {
            fc.autostart.timestamp = HAL_GetTick();
            fc.autostart.altitude = fc.fused.altitude1;
        }

    }

    if (fc.flight.mode == flight_flight)
    {
        bool check = false;

        if (config_get_bool(&profile.flight.auto_landing.alt_change_enabled))
        {
            if (abs(fc.autostart.altitude - fc.fused.altitude1) >
                config_get_int(&profile.flight.auto_landing.alt_change_value))
            {
                fc.autostart.timestamp = HAL_GetTick();
                fc.autostart.altitude = fc.fused.altitude1;
            }
            check = true;
        }

        if (config_get_bool(&profile.flight.auto_landing.speed_enabled))
        {
            if (fc.gnss.fix == 3)
            {
                if (fc.gnss.ground_speed >
                    config_get_int(&profile.flight.auto_landing.speed_value))
                {
                    fc.autostart.timestamp = HAL_GetTick();
                }
                check = true;
            }
        }

        if (check)
        {
            uint32_t delta = HAL_GetTick() - fc.autostart.timestamp;
            if (delta > config_get_int(&profile.flight.auto_landing.timeout) * 1000)
            {
                fc_landing();
            }
        }

    }

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
	int32_t delta = timezone_get_offset(config_get_select(&config.time.zone));
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

