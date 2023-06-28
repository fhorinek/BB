//#define DEBUG_LEVEL	DBG_DEBUG
#include "csv.h"

#include "fc/fc.h"
#include "etc/epoch.h"
#include "etc/timezone.h"
#include "drivers/rev.h"
#include "drivers/rtc.h"

static osTimerId_t csv_timer;

#define CSV_PERIOD	100

int32_t csv_log_file;
static bool csv_started = false;
static uint32_t csv_ticks = 0;

// This could also be realized as a function to save PROGMEM
#define csv_write_line(LINE) red_write_line(csv_log_file, LINE, LINE_ENDING_CRLF)

void csv_write_data()
{
	char line[100];

	uint32_t timestamp = (fc.gnss.fix == 0) ? fc_get_utc_time() : fc.gnss.utc_time;
	//ms_since_start,timestamp,latitude,longtitude,ground_speed,heading,gnss_alt,baro_alt,vario,accel
	snprintf(line, sizeof(line), "%lu,%lu,%0.7f,%0.7f,%0.1f,%0.0f,%d,%d,%0.0f,%0.3f",
			(HAL_GetTick() - csv_ticks),
			timestamp,
			(float) fc.gnss.latitude / GNSS_MUL,
			(float) fc.gnss.longitude / GNSS_MUL,
			fc.gnss.ground_speed,
			fc.gnss.heading,
			(fc.gnss.fix == 3 ? (int16_t) fc.gnss.altitude_above_ellipsiod : 0),
			(fc.fused.status == fc_dev_ready ? (int16_t) fc_press_to_alt(fc.fused.pressure, 101325) : 0),
			(fc.fused.status == fc_dev_ready ? fc.fused.vario * 100 : 0),
			(fc.imu.status == fc_dev_ready ? fc.imu.acc_total : 0)
			);
	csv_write_line(line);
}

void csv_start_write()
{
	char line[100];

	//create file
	char path[PATH_LEN];

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;
	uint64_t utc_time = fc_get_utc_time();
	DBG("utc_time %lu", utc_time);
	datetime_from_epoch(utc_time, &sec, &min, &hour, &day, &wday, &month, &year);

	snprintf(path, sizeof(path), "%s/%04u.%02u", PATH_LOGS_DIR, year, month);
	red_mkdir(path);
	snprintf(path, sizeof(path), "%s/%04u.%02u/%04u.%02u.%02u %02u.%02u.csv", PATH_LOGS_DIR, year, month, year, month, day, hour, min);
	csv_log_file = red_open(path, RED_O_WRONLY | RED_O_CREAT | RED_O_TRUNC);

	DBG("CSV OPEN %s, res = %u", path, csv_log_file);
	ASSERT(csv_log_file > 0);
	if (csv_log_file < 0)
		return;

	sprintf(line, "ms_since_start,timestamp,latitude,longtitude,ground_speed,heading,gnss_alt,baro_alt,vario,accel");
	csv_write_line(line);
	csv_ticks = HAL_GetTick();
}


void csv_tick_cb(void * arg)
{
	UNUSED(arg);

	if (csv_started)
	{
		csv_write_data();
		red_sync();
	}
	else {
		if (rtc_is_valid())
		{
			csv_start_write();
			fc.logger.csv = fc_logger_record;
			csv_started = true;
		}
	}
}

void csv_start()
{
	DBG("CSV timer start");
    osTimerStart(csv_timer, CSV_PERIOD);
    fc.logger.csv = fc_logger_wait;
}

void csv_init()
{
	csv_started = false;
	csv_timer = osTimerNew(csv_tick_cb, osTimerPeriodic, NULL, NULL);

	fc.logger.csv = fc_logger_off;
}

void csv_stop()
{
	osTimerStop(csv_timer);

	if (csv_started)
	{
		red_close(csv_log_file);
		csv_started = false;
	}
	fc.logger.csv = fc_logger_off;
}
