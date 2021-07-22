/*
 * fc.cc
 *
 *  Created on: May 6, 2020
 *      Author: horinek
 */
#define DEBUG_LEVEL	DEBUG_DBG

#include "fc.h"

#include "kalman.h"

#include "etc/epoch.h"
#include "etc/timezone.h"

#include "drivers/rtc.h"
#include "fc/vario.h"
#include "fc/logger/logger.h"

fc_t fc;

void fc_history_record_cb(void * arg)
{
//	DBG("fc_history_record_cb");

	fc_pos_history_t pos;
	memset(&pos, 0, sizeof(pos));

	if (fc.fused.status == fc_dev_ready)
	{
		pos.flags |= FC_POS_HAVE_BARO;
		pos.baro_alt = fc_press_to_alt(fc.fused.pressure, 101325);
		pos.vario = fc.fused.vario * 100;
	}
	else
	{
		return;
	}

	if (fc.gnss.fix > 0 && fc.gnss.status == fc_dev_ready)
	{
		pos.lat = fc.gnss.latitude;
		pos.lon = fc.gnss.longtitude;
		pos.ground_hdg = fc.gnss.heading;
		pos.ground_spd = fc.gnss.ground_speed * 100;
		if (fc.gnss.fix == 3)
		{
			pos.gnss_alt = fc.gnss.altitude_above_ellipsiod;
			pos.flags |= FC_POS_GNSS_3D;
		}
		else
		{
			pos.flags |= FC_POS_GNSS_2D;
		}
	}
	else
	{
		pos.flags |= FC_POS_NO_GNSS;
	}

	FC_ATOMIC_ACCESS
	{
		memcpy(&fc.history.positions[fc.history.index], &pos, sizeof(pos));
		fc.history.index = (fc.history.index + 1) % FC_HISTORY_SIZE;

		if (fc.history.size < FC_HISTORY_SIZE)
			fc.history.size++;
	}
}


void fc_reset()
{
    if (fc.fused.status != fc_dev_ready)
    {
        fc.flight.mode = flight_not_ready;
    }
    else
    {
        fc.autostart.altitude = fc.fused.altitude1;
        fc.autostart.timestamp = HAL_GetTick();
    }

    fc.history.index = 0;
    fc.history.size = 0;
}

void fc_init()
{
    //create released semaphore
    fc.lock = osSemaphoreNew(1, 1, NULL);
    vQueueAddToRegistry(fc.lock, "fc.lock");

	INFO("Flight computer init");

	vario_profile_load(config_get_text(&profile.vario.profile));

	fc.history.positions = (fc_pos_history_t *) ps_malloc(sizeof(fc_pos_history_t) * FC_HISTORY_SIZE);
	fc.history.timer = osTimerNew(fc_history_record_cb, osTimerPeriodic, NULL, NULL);

    osTimerStart(fc.history.timer, FC_HISTORY_PERIOD);


	fc_reset();
	logger_init();
}

void fc_deinit()
{
	INFO("Flight computer deinit");
	logger_stop();
	osTimerStop(fc.history.timer);
}

void fc_takeoff()
{
    INFO("Take-off");
    fc.flight.start_alt = fc.fused.altitude1;
    fc.flight.start_time = HAL_GetTick();

    fc.autostart.timestamp = HAL_GetTick();
    fc.autostart.altitude = fc.fused.altitude1;

    fc.flight.mode = flight_flight;

    logger_start();
}

void fc_landing()
{
    INFO("Landing");
    fc.flight.duration = (HAL_GetTick() - fc.flight.start_time) / 1000;
    fc.flight.mode = flight_landed;

    fc_reset();

    logger_stop();
}

void fc_step()
{
    if (fc.flight.mode == flight_wait_to_takeoff
    		|| fc.flight.mode == flight_landed)
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
	int32_t delta = timezone_get_offset(config_get_select(&config.time.zone), config_get_bool(&config.time.dst));
	fc_set_time(datetime + delta);
}

uint64_t fc_get_utc_time()
{
	int32_t delta = timezone_get_offset(config_get_select(&config.time.zone), config_get_bool(&config.time.dst));
	return rtc_get_epoch() - delta;
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

void fc_manual_alt1_change(float val)
{
	kalman_configure(val);

    if (fc.flight.mode == flight_wait_to_takeoff)
    	fc.autostart.altitude = val;
}
