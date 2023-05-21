/*
 * fc.cc
 *
 *  Created on: May 6, 2020
 *      Author: horinek
 */
#define DEBUG_LEVEL	DBG_DEBUG

#include <inttypes.h>

#include "fc.h"

#include "kalman.h"

#include "etc/epoch.h"
#include "etc/timezone.h"

#include "drivers/rtc.h"
#include "drivers/gnss/fanet.h"

#include "fc/telemetry/telemetry.h"
#include "fc/vario.h"
#include "fc/logger/logger.h"
#include "fc/agl.h"
#include "fc/navigation.h"
#include "fc/circling.h"
#include "fc/wind.h"
#include "fc/recorder.h"

#include "gui/tasks/page/pages.h"
#include "etc/notifications.h"

fc_t fc;

static TaskHandle_t fc_lock_owner = NULL;

bool fc_lock_acquire()
{
    uint32_t start = HAL_GetTick();

    uint32_t wait = (fc_lock_owner == NULL) ? WAIT_INF : 3000;

    TaskHandle_t prev_lock_owner = fc_lock_owner;

    osStatus_t stat = osMutexAcquire(fc.lock, wait);
    if (stat == osErrorTimeout)
    {
        bsod_msg("Not able to acquire fc.lock in time from task '%s' blocked by task '%s'!",
                pcTaskGetName(xTaskGetCurrentTaskHandle()), pcTaskGetName(fc_lock_owner));
    }
    uint32_t delta = HAL_GetTick() - start;
    if (delta > 100 && prev_lock_owner != NULL)
    {
        WARN("'%s' was unable to acquire fc.lock from '%s' for %u ms!",
                pcTaskGetName(xTaskGetCurrentTaskHandle()), pcTaskGetName(prev_lock_owner), delta);
    }
    fc_lock_owner = xTaskGetCurrentTaskHandle();

    return true;
}

bool fc_lock_release()
{
    fc_lock_owner = NULL;
    osMutexRelease(fc.lock);

    return false;
}

void fc_history_record_cb(void *arg)
{
//	DBG("fc_history_record_cb");

    __align fc_pos_history_t pos;
    memset(&pos, 0, sizeof(pos));

    if (fc.fused.status == fc_dev_ready)
    {
        pos.flags |= FC_POS_HAVE_BARO;
        pos.baro_alt = fc_press_to_alt(fc.fused.pressure, 101325);
        pos.vario = fc.fused.vario * 100;
    }

    if (fc.gnss.fix > 0 && fc.gnss.status == fc_dev_ready)
    {
        pos.lat = fc.gnss.latitude;
        pos.lon = fc.gnss.longitude;
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

    if (fc.imu.status == fc_dev_ready)
    {
        pos.flags |= FC_POS_HAVE_ACC;
        pos.accel = fc.imu.acc_total * 1000;
    }

    FC_ATOMIC_ACCESS
        {
        safe_memcpy(&fc.history.positions[fc.history.index], &pos, sizeof(pos));
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

    if (fc.flight.mode != flight_landed)
    {
        fc.flight.start_lat = INVALID_INT32;
        fc.flight.start_lon = INVALID_INT32;
    }

    fc.history.index = 0;
    fc.history.size = 0;

    fc.fused.glide_ratio = NAN;
    fc.agl.ground_height = AGL_INVALID;
    fc.agl.agl = AGL_INVALID;

    fc.flight.odometer = 0;
    fc.flight.max_alt = -32678;
    fc.flight.min_alt = 32677;
    fc.flight.max_climb = 0;
    fc.flight.max_sink = 0;
    fc.flight.min_lat = INT32_MAX;
    fc.flight.max_lat = INT32_MIN;
    fc.flight.min_lon = INT32_MAX;
    fc.flight.max_lon = INT32_MIN;

    circling_reset();
}

static osTimerId_t fc_timer;

void fc_init()
{
    //create released semaphore
    fc.lock = osMutexNew(NULL);
    vQueueAddToRegistry(fc.lock, "fc.lock");

    INFO("Flight computer init");

    vario_profile_load(config_get_text(&profile.vario.profile));

    fc.airspaces.near.asn = NULL;
    fc.airspaces.near.size = 0;
    fc.airspaces.near.num = 0;
    fc.airspaces.near.valid = false;
    fc.airspaces.near.last_updated = 0;
    fc.airspaces.near.used_heading = -999;
    fc.airspaces.near.used_pilot_pos.latitude = 0;
    fc.airspaces.near.used_pilot_pos.longitude = 0;

    fc.history.positions = (fc_pos_history_t*) ps_malloc(sizeof(fc_pos_history_t) * FC_HISTORY_SIZE);
    fc.history.timer = osTimerNew(fc_history_record_cb, osTimerPeriodic, NULL, NULL);

    if (config_get_bool(&profile.audio.thermal_fade))
    {
        fc.esp.vario_volume = config_get_int(&profile.audio.vario_volume);
        fc.esp.a2dp_volume = config_get_int(&profile.audio.a2dp_thermal_volume);
    }
    else
    {
        fc.esp.vario_volume = config_get_int(&profile.audio.vario_volume);
        fc.esp.a2dp_volume = config_get_int(&profile.audio.a2dp_volume);
    }

    osTimerStart(fc.history.timer, FC_HISTORY_PERIOD);

    fc_recorder_init();
	fc_reset();
	logger_init();
	telemetry_init();
	wind_init();

    fc_timer = osTimerNew(fc_step, osTimerPeriodic, NULL, NULL);
    osTimerStart(fc_timer, FC_STEP_PERIOD);

}

void fc_deinit()
{
	INFO("Flight computer deinit");
	logger_stop();
	telemetry_stop();
	osTimerStop(fc.history.timer);
	osTimerStop(fc_timer);
	fc_recorder_exit();
}

void fc_takeoff()
{
    INFO("Take-off");
    fc.flight.start_alt = fc.fused.altitude1;
    fc.flight.start_time = HAL_GetTick();

    if (fc.gnss.fix == 3)
    {
        fc.flight.start_lat = fc.gnss.latitude;
        fc.flight.start_lon = fc.gnss.longitude;
        fc.flight.takeoff_distance = 0;
    }
    else
    {
        fc.flight.start_lat = INVALID_INT32;
        fc.flight.start_lon = INVALID_INT32;
        fc.flight.takeoff_distance = INVALID_UINT32;
    }

    if (config_get_bool(&profile.wifi.off_in_flight))
    {
        esp_set_wifi(false);
    }

    fc.autostart.timestamp = HAL_GetTick();
    fc.autostart.altitude = fc.fused.altitude1;

    fc.flight.mode = flight_flight;

    fanet_set_mode(false);
    logger_start();
    fc_recorder_reset();

    gui_page_set_next(&profile.ui.autoset.take_off);

    notification_send(notify_take_off);
}

void fc_save_stats()
{
    // fc_get_utc_time() casted to uint32_t as ARM's printf does not support 64 bit integer.
    flight_stats_t f_stat;

    f_stat.start_time = (uint32_t) fc_get_utc_time() - fc.flight.duration;
    f_stat.tz_offset = timezone_get_offset(config_get_select(&config.time.zone), config_get_bool(&config.time.dst));
    f_stat.duration = fc.flight.duration;
    f_stat.max_alt = fc.flight.max_alt;
    f_stat.min_alt = fc.flight.min_alt;
    f_stat.max_climb = fc.flight.max_climb;
    f_stat.max_sink = fc.flight.max_sink;
    f_stat.odo = fc.flight.odometer / 100;     // cm to m
    f_stat.min_lat = fc.flight.min_lat;
    f_stat.max_lat = fc.flight.max_lat;
    f_stat.min_lon = fc.flight.min_lon;
    f_stat.max_lon = fc.flight.max_lon;

    logger_write_flight_stats(f_stat);

}

void fc_landing()
{
    INFO("Landing");
    fc.flight.duration = (HAL_GetTick() - fc.flight.start_time) / 1000;
    fc.flight.mode = flight_landed;
    fc_save_stats();

    fc_reset();

    fanet_set_mode(false);
    logger_stop();

    if (config_get_bool(&profile.wifi.off_in_flight))
    {
        esp_set_wifi(true);
    }

    gui_page_set_next(&profile.ui.autoset.land);

    notification_send(notify_landing);
}

//run via timer every 250ms
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
                if (!fc.autostart.wait_for_manual_change)
                    fc_takeoff();
            }
        }

        if (config_get_bool(&profile.flight.auto_take_off.speed_enabled))
        {
            if (fc.gnss.fix == 3)
            {
                if (fc.gnss.ground_speed_calm > GROUND_SPEED_CALM_CNT)
                {
                    if (fc.gnss.ground_speed >
                            config_get_int(&profile.flight.auto_take_off.speed_value))
                    {
                        fc_takeoff();
                    }
                }
                else
                {
                    if (fc.gnss.ground_speed < config_get_int(&profile.flight.auto_take_off.speed_value))
                        fc.gnss.ground_speed_calm++;
                    else
                        fc.gnss.ground_speed_calm = 0;
                }
            }
            else
            {
                fc.gnss.ground_speed_calm = 0;
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

        if (fc.fused.altitude1 > fc.flight.max_alt)
            fc.flight.max_alt = fc.fused.altitude1;

        if (fc.fused.altitude1 < fc.flight.min_alt)
            fc.flight.min_alt = fc.fused.altitude1;

        int16_t t_vario = fc.fused.vario * 100;         // meter/s -> cm/s
        if (t_vario > fc.flight.max_climb) 	fc.flight.max_climb = t_vario;
        if (t_vario < fc.flight.max_sink) 	fc.flight.max_sink = t_vario;
        if (fc.gnss.fix == 3)
		{
			fc.flight.min_lat = min(fc.flight.min_lat, fc.gnss.latitude);
			fc.flight.max_lat = max(fc.flight.max_lat, fc.gnss.latitude);
			fc.flight.min_lon = min(fc.flight.min_lon, fc.gnss.latitude);
			fc.flight.max_lon = max(fc.flight.max_lon, fc.gnss.longitude);
	        fc_recorder_step(fc.gnss.latitude, fc.gnss.longitude, (int16_t)fc.fused.altitude1);
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

    //glide ratio
    //when you have GNSS, baro and speed is higher than 2km/h and you are sinking <= -0.01
    if (fc.gnss.fix == 3
            && fc.fused.status == fc_dev_ready
            && fc.gnss.ground_speed > FC_GLIDE_MIN_SPEED
            && fc.fused.gr_vario <= FC_GLIDE_MIN_SINK)
    {
        fc.fused.glide_ratio = fc.gnss.ground_speed / abs(fc.fused.gr_vario);
    }
    else
    {
        fc.fused.glide_ratio = NAN;
    }

    //vario volume fade
    if (fc.esp.vario_volume_last_change < HAL_GetTick())
    {
        uint8_t vario_target = config_get_int(&profile.audio.vario_glide_volume);
        uint8_t a2dp_target = config_get_int(&profile.audio.a2dp_volume);

        bool pass = true;
        if (config_get_bool(&profile.audio.thermal_connected))
        {
            if (!(fc.esp.state & ESP_STATE_BT_A2DP))
            {
                pass = false;
            }
        }

        if (config_get_bool(&profile.audio.thermal_fade) && pass)
        {
            //Enabled & connected (id needed)

            if (fc.flight.mode == flight_flight)
            {
                int16_t ivario = fc.fused.vario * 10;

                if (ivario > config_get_int(&profile.audio.idle_max) ||
                        ivario < config_get_int(&profile.audio.idle_min))
                {
                    vario_target = config_get_int(&profile.audio.vario_volume);
                    a2dp_target = config_get_int(&profile.audio.a2dp_thermal_volume);
                }
            }
        }
        else
        {
            //default
            vario_target = config_get_int(&profile.audio.vario_volume);
            a2dp_target = config_get_int(&profile.audio.a2dp_volume);
        }

        int8_t change = config_get_int(&profile.audio.change_spd);

        if (fc.esp.vario_volume != vario_target)
        {
            int16_t diff = vario_target - fc.esp.vario_volume;

            if (diff > 0)
                fc.esp.vario_volume += min(change, diff);
            else
                fc.esp.vario_volume += max(-change, diff);

            esp_set_volume(PROTO_VOLUME_VARIO, fc.esp.vario_volume);
        }

        if (fc.esp.a2dp_volume != a2dp_target)
        {
            int16_t diff = a2dp_target - fc.esp.a2dp_volume;

            if (diff > 0)
                fc.esp.a2dp_volume += min(change, diff);
            else
                fc.esp.a2dp_volume += max(-change, diff);

            esp_set_volume(PROTO_VOLUME_A2DP, fc.esp.a2dp_volume);
        }

        fc.esp.vario_volume_last_change = HAL_GetTick() + 200;

        //DBG("VA %u A2 %u", fc.esp.vario_volume, fc.esp.a2dp_volume);
    }

    navigation_step();
    circling_step();
    agl_step();
    wind_step();
}

void fc_device_status(char *buff, fc_device_status_t status)
{
    switch (status)
    {
        case (fc_dev_init):
            strcpy(buff, _("Device init"));
        break;

        case (fc_dev_ready):
            strcpy(buff, _("Device ready"));
        break;

        case (fc_dev_error):
            strcpy(buff, _("Device error"));
        break;

        case (fc_device_not_calibrated):
            strcpy(buff, _("Not calibrated"));
        break;

        case (fc_dev_off):
            strcpy(buff, _("Device disabled"));
        break;

        default:
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
    int64_t delta = timezone_get_offset(config_get_select(&config.time.zone), config_get_bool(&config.time.dst));
    fc_set_time(datetime + delta);
}

uint64_t fc_get_utc_time()
{
    int64_t delta = timezone_get_offset(config_get_select(&config.time.zone), config_get_bool(&config.time.dst));
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

    if (fc.flight.mode != flight_flight)
    {
        fc.autostart.altitude = val;
        fc.autostart.wait_for_manual_change = true;
    }
}
