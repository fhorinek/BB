/*
 * kml.c: a logger for KML files
 *
 *  Created on: Jun 27, 2023
 *      Author: tilmann@bubecks.de
 */

//#define DEBUG_LEVEL	DBG_DEBUG
#include "kml.h"

#include "fc/fc.h"
#include "etc/epoch.h"
#include "etc/timezone.h"
#include "drivers/rev.h"
#include "drivers/rtc.h"

static osTimerId_t kml_timer;

#define KML_PERIOD	900

static int32_t kml_log_file = 0;
static bool kml_started = false;
static uint32_t last_timestamp = 0;

// This could also be realized as a function to save PROGMEM
#define kml_write_line(LINE) red_write_line(kml_log_file, LINE, LINE_ENDING_CRLF)

/**
 * Here we save the filepointer at the position right before the
 * <TimeStamp>...<end>". So we can rewind to set the end date, when
 * we close the log.
 */
static uint32_t filepos_for_end;

/**
 * Insert a line into the log containing a XML element and "now" as the content,
 * e.g. "<end>2016-12-24T18:00:00Z</end>".
 *
 * \param tag the name of the XML element, in the above example, this would be "end".
 */
static void kml_now(const char *tag) {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;
	char line[80];

	datetime_from_epoch(fc.gnss.utc_time, &sec, &min, &hour, &day, &wday, &month, &year);
	sprintf(line, "<%s>%04d-%02d-%02dT%02d:%02d:%02dZ</%s>", tag, year, month, day, hour, min, sec, tag);
	kml_write_line(line);
}

static void kml_write_pos(uint8_t fix, int32_t longitude, int32_t latitude, float altitude)
{
	char line[100];

	if (fix >= 2 )
	{
		char tmp1[16];
		char tmp2[16];

		sprintf(tmp1, " %+011ld", longitude);
		memmove((void *)tmp1, (void *)(tmp1 + 1), 4);
		tmp1[4] = '.';

		sprintf(tmp2, " %+010ld", latitude);
		memmove((void *)tmp2, (void *)(tmp2 + 1), 3);
		tmp2[3] = '.';

		sprintf(line, "%s,%s,%0.0f", tmp1, tmp2, altitude);
		kml_write_line(line);
	}
}

static void kml_start_write()
{
	char line[100];
	uint8_t fix;

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
	snprintf(path, sizeof(path), "%s/%04u.%02u/%04u.%02u.%02u %02u.%02u.kml", PATH_LOGS_DIR, year, month, year, month, day, hour, min);
	kml_log_file = red_open(path, RED_O_WRONLY | RED_O_CREAT | RED_O_TRUNC);

	DBG("KML OPEN %s, res = %u", path, kml_log_file);
	ASSERT(kml_log_file > 0);
	if (kml_log_file < 0)
		return;

	//A record
	uint8_t device_id[12];
	rev_get_uuid(device_id);
	char serial_number[23];

	for (uint8_t i = 0; i < 11; i++)
		sprintf(serial_number + (i * 2), "%02X", device_id[i]);

	kml_write_line("<kml xmlns=\"http://www.opengis.net/kml/2.2\">");
	kml_write_line("<Document>");
	sprintf(line, "<name>Flight %02d.%02d.%04d @ %02d:%02d</name>", day, month, year, hour, min);
	kml_write_line(line);
	sprintf(line, "<Placemark id=\"%s-%lu\">", serial_number, (uint32_t)utc_time);
	kml_write_line(line);
	kml_write_line("<name>Flight</name>");
	kml_write_line("<visibility>1</visibility>");
	kml_write_line("<open>1</open>");

	kml_write_line("<TimeSpan>");
	kml_now("begin");
	// Save position of end date, so that we can overwrite on close:
    filepos_for_end = red_lseek(kml_log_file, 0, RED_SEEK_CUR);

	kml_now("end");
	kml_write_line("</TimeSpan>");

	kml_write_line("<Style>");
	kml_write_line("<LineStyle><color>ff00ffff</color></LineStyle>");
	kml_write_line("<PolyStyle><color>7f0000ff</color></PolyStyle>");
	kml_write_line("</Style>");
	kml_write_line("<LineString>");
	kml_write_line("<extrude>1</extrude>");
	kml_write_line("<altitudeMode>absolute</altitudeMode>");
	kml_write_line("<coordinates>");

	//write buffer
	FC_ATOMIC_ACCESS
	{
		uint8_t step = 1000 / FC_HISTORY_PERIOD;

		uint16_t start = fc.history.size / step;
		uint16_t end = max(0, start - (60 * step));
		for (int16_t i = start; i > end; i--)
		{
			uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i * step) % FC_HISTORY_SIZE;

			fc_pos_history_t * pos = &fc.history.positions[index];

			if ( pos->flags & FC_POS_GNSS_3D )
				fix = 3;
			else if ( pos->flags & FC_POS_GNSS_2D )
				fix = 2;
			else
				fix = 0;
			kml_write_pos(fix, pos->lon, pos->lat, pos->baro_alt);
		}
	}

	kml_comment("pre flight buffer end");
}

static void kml_tick_cb(void * arg)
{
	UNUSED(arg);

	if (kml_started)
	{
		uint32_t timestamp = (fc.gnss.fix == 0) ? fc_get_utc_time() : fc.gnss.utc_time;

		if ((last_timestamp >= timestamp) && (abs(last_timestamp - timestamp) < 10))
		{
			DBG("KML last_timestamp %lu, timestamp %lu", last_timestamp, timestamp);
			return;
		}

		last_timestamp = timestamp;

		kml_write_pos(fc.gnss.fix, fc.gnss.longitude, fc.gnss.latitude, fc.fused.altitude1);
		red_sync();
	}
	else {
		if (rtc_is_valid())
		{
			kml_start_write();
			fc.logger.kml = fc_logger_record;
			kml_started = true;
		}
	}
}

void kml_start()
{
	DBG("KML timer start");
    osTimerStart(kml_timer, KML_PERIOD);
    fc.logger.kml = fc_logger_wait;
}

void kml_comment(char * text)
{
	char line[100];

	if (kml_started)
	{
		snprintf(line, sizeof(line), "<!-- %s -->", text);
		kml_write_line(line);
	}
}

void kml_init()
{
	kml_started = false;
	kml_timer = osTimerNew(kml_tick_cb, osTimerPeriodic, NULL, NULL);

	fc.logger.kml = fc_logger_off;
}

void kml_stop()
{
	osTimerStop(kml_timer);

	if (kml_started)
	{
		kml_write_line("</coordinates>");
		kml_write_line("</LineString>");
		kml_write_line("</Placemark>");
		kml_write_line("</Document>");
		kml_write_line("</kml>");

		// Overwrite previous entry of <end> with "now":
		red_lseek(kml_log_file, filepos_for_end, RED_SEEK_SET);
		kml_now("end");

		red_close(kml_log_file);
		kml_log_file = 0;             // Safety invalidation
		kml_started = false;
	}
	fc.logger.kml = fc_logger_off;
}
