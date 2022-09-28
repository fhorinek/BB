#define DEBUG_LEVEL	DEBUG_DBG

#include "igc.h"

#include "fc/fc.h"
#include "sha256.h"
#include "etc/epoch.h"
#include "etc/timezone.h"
#include "etc/geo_calc.h"
#include "drivers/rev.h"
#include "drivers/rtc.h"

#include <private_key.h>

static osTimerId_t igc_timer;

#define IGC_PERIOD	900
#define LOG_IGC_MANUFACTURER_ID	"XSB"
#define LOG_IGC_DEVICE_ID		"DRP"
//#define LOG_IGC_DEVICE_ID		"STR"

int32_t igc_log_file;
static bool igc_started = false;

sha_internal_state_t sha_state;

void igc_writeline_2(char * line, bool sign)
{
	uint8_t l = strlen(line);

	DBG("IGC:%s", line);

	char new_line[l + 3];
	snprintf(new_line, sizeof(new_line), "%s\r\n", line);
	l += 2;

	ASSERT(red_write(igc_log_file, new_line, l) == l);
	ASSERT(red_sync() == 0);

#ifndef IGC_NO_PRIVATE_KEY
	if (sign)
	{
		for (uint8_t i = 0; i < l; i++)
		{
			sha256_write(&sha_state, new_line[i]);
		}
	}
#endif
}

void igc_writeline(char * line)
{
	igc_writeline_2(line, true);
}


void igc_write_grecord()
{
#ifndef IGC_NO_PRIVATE_KEY

	if (fc.gnss.fake)
		return;

	char line[79];

	__align sha_internal_state_t tmp_sha;
	safe_memcpy(&tmp_sha, &sha_state, sizeof(sha_state));

	//G record
	uint8_t * res = sha256_result(&tmp_sha);
	strcpy(line, "G");
	for (uint8_t i = 0; i < 20; i++)
	{
		char tmp[3];

		sprintf(tmp, "%02X", res[i]);
		strcat(line, tmp);
	}

	int32_t pos = red_lseek(igc_log_file, 0, RED_SEEK_CUR);
	igc_writeline_2(line, false);
	red_sync();

	//rewind pointer
	red_lseek(igc_log_file, pos, RED_SEEK_SET);
#endif
}

void igc_comment(char * text)
{
	char line[79];

	snprintf(line, sizeof(line), "L%s %s", LOG_IGC_MANUFACTURER_ID, text);
	igc_writeline_2(line, false);
	igc_write_grecord();
}

static uint32_t last_timestamp = 0;

void igc_write_b(uint32_t timestamp, int32_t lat, int32_t lon, int16_t gnss_alt, bool valid, int16_t baro_alt)
{
	char line[79];

	uint8_t sec;
	uint8_t min;
	uint8_t hour;

	time_from_epoch(timestamp, &sec, &min, &hour);

	char slat[16];
	char slon[16];

	uint32_t alat = abs(lat);
	uint32_t alon = abs(lon);
	uint32_t mlat = ((alat % GNSS_MUL) * 60);
	uint32_t mlon = ((alon % GNSS_MUL) * 60);
	uint32_t mlat1 = mlat / GNSS_MUL;
	uint32_t mlon1 = mlon / GNSS_MUL;
	uint32_t mlat2 = (mlat % GNSS_MUL) / 10000;
	uint32_t mlon2 = (mlon % GNSS_MUL) / 10000;


	snprintf(slat, sizeof(slat), "%02lu%02lu%03lu%c", alat / GNSS_MUL, mlat1, mlat2, lat > 0 ? 'N' : 'S');
	snprintf(slon, sizeof(slon), "%03lu%02lu%03lu%c", alon / GNSS_MUL, mlon1, mlon2, lon > 0 ? 'E' : 'W');

//	DBG("lat %ld %lu %lu %lu %lu '%s'", lat, alat, mlat, mlat1, mlat2, slat);
//	DBG("lon %ld %lu %lu %lu %lu '%s'", lon, alon, mlon, mlon1, mlon2, slon);

	char c = (valid) ? 'A' : 'V';

	snprintf(line, sizeof(line), "B%02u%02u%02u%s%s%c%05d%05d", hour, min, sec, slat, slon, c, baro_alt, gnss_alt);
	igc_writeline(line);
	igc_write_grecord();
}

IGC_PRIVATE_KEY_BODY;

void igc_start_write()
{
	sha256_init(&sha_state);

	uint8_t device_id[12];
	rev_get_uuid(device_id);

	IGC_PRIVATE_KEY_ADD;

	char line[79];

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
	snprintf(path, sizeof(path), "%s/%04u.%02u/%04u.%02u.%02u %02u.%02u.igc", PATH_LOGS_DIR, year, month, year, month, day, hour, min);
	igc_log_file = red_open(path, RED_O_WRONLY | RED_O_CREAT);
	DBG("IGC OPEN %s, res = %d", path, igc_log_file);
	if (igc_log_file < 0)
		return;

	//write header

	//A record
	char serial_number[23];

	for (uint8_t i = 0; i < 11; i++)
		sprintf(serial_number + (i * 2), "%02X", device_id[i]);

	sprintf(line, "A%s%s:%s", LOG_IGC_MANUFACTURER_ID, LOG_IGC_DEVICE_ID, serial_number);
	igc_writeline(line);
	//H records
	//H F DTE
	sprintf(line, "HFDTE%02u%02u%02u", day, month, year % 100);
	igc_writeline(line);
	//H F PLT PILOT IN CHARGE
	sprintf(line, "HFPLTPILOTINCHARGE:%s", config_get_text(&pilot.name));
	igc_writeline(line);
	//H F CM2 CREW 2
	sprintf(line, "HFCM2CREW2:NIL");
	igc_writeline(line);
	//H F GTY GLIDER TYPE
	sprintf(line, "HFGTYGLIDERTYPE:%s", config_get_text(&pilot.glider_type));
	igc_writeline(line);
	//H F GID GLIDER ID
	sprintf(line, "HFGIDGLIDERID:%s", config_get_text(&pilot.glider_id));
	igc_writeline(line);
	//H F DTM GPS DATUM
	sprintf(line, "HFDTMGPSDATUM:WGS84");
	igc_writeline(line);
	//H F RFW FIRMWARE VERSION
	char sw[20];
	rev_get_sw_string(sw);
	sprintf(line, "HFRFWFIRMWAREVERSION:%s", sw);
	igc_writeline(line);
	//H F RHW HARDWARE VERSION
	sprintf(line, "HFRHWHARDWAREVERSION:strato_%02X", rev_get_hw());
	igc_writeline(line);
	//H F FTY FR TYPE
	sprintf(line, "HFFTYFRTYPE:SkyBean,Strato");
	igc_writeline(line);
	//H F GPS RECEIVER
	sprintf(line, "HFGPSRECEIVER:u-blox,NEO-M8Q,22cm,18000m");
	igc_writeline(line);
	//H F PRS PRESS ALT SENSOR
	sprintf(line, "HFPRSPRESSALTSENSOR:Measurement specialties,MS5611,25907m");
	igc_writeline(line);
	//H F ALG ALT GPS
	sprintf(line, "HFALGALTGPS:GEO");
	igc_writeline(line);
	//H F ALP
	sprintf(line, "HFALPALTPRESSURE:ISA");
	igc_writeline(line);
	//H F TZN
	int32_t delta = timezone_get_offset(config_get_select(&config.time.zone), config_get_bool(&config.time.dst));
	sprintf(line, "HFTZNTIMEZONE:%+0.1f", delta / 3600.0);
	igc_writeline(line);

#ifdef IGC_NO_PRIVATE_KEY
	//Developer note: we can't publish the private key for signing the IGC file

	//H F FRS
	sprintf(line, "HFFRSSECSUSPECTUSEVALIPROG:This file is not valid. Private key not available!");
	igc_writeline(line);
#endif

	igc_comment("pre flight buffer start");

	//write buffer
	FC_ATOMIC_ACCESS
	{
		uint8_t step = 1000 / FC_HISTORY_PERIOD;

		uint32_t now = fc_get_utc_time();
		uint16_t start = fc.history.size / step;
		uint16_t end = max(0, start - (60 * step));
		for (int16_t i = start; i > end; i--)
		{
			uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i * step) % FC_HISTORY_SIZE;

			fc_pos_history_t * pos = &fc.history.positions[index];

			bool valid = pos->flags & FC_POS_GNSS_3D;
			if ( valid ) 
			  igc_write_b(now - i, pos->lat, pos->lon, pos->gnss_alt, valid, pos->baro_alt);
		}
	}

	igc_comment("pre flight buffer end");


}

void igc_tick_cb(void * arg)
{
	if (igc_started)
	{
		//write B record

		uint32_t timestamp = (fc.gnss.fix == 0) ? fc_get_utc_time() : fc.gnss.utc_time;

		if ((last_timestamp >= timestamp) && (abs(last_timestamp - timestamp) < 10))
		{
			DBG("last_timestamp %lu, timestamp %lu", last_timestamp, timestamp);
			return;
		}

		last_timestamp = timestamp;

		bool valid = (fc.gnss.fix == 3);

		igc_write_b(timestamp, fc.gnss.latitude, fc.gnss.longtitude, fc.gnss.altitude_above_ellipsiod, valid, fc_press_to_alt(fc.fused.pressure, 101325));
	}
	else
	{
		if (rtc_is_valid())
		{
			igc_start_write();
			fc.logger.igc = fc_logger_record;
			igc_started = true;
		}
	}
}

void igc_init()
{
	igc_started = false;
	igc_timer = osTimerNew(igc_tick_cb, osTimerPeriodic, NULL, NULL);

	fc.logger.igc = fc_logger_off;
}


void igc_start()
{
	DBG("IGC timer start");
    osTimerStart(igc_timer, IGC_PERIOD);
    fc.logger.igc = fc_logger_wait;
}

void igc_stop()
{
	osTimerStop(igc_timer);

	if (igc_started)
	{
		igc_comment("end");
		red_close(igc_log_file);
		igc_started = false;
	}
	fc.logger.igc = fc_logger_off;
}

/**
 * Read the next B record from a IGC file and return the position.
 *
 * @param igc_log_read_file a file handle of the log file to parse
 * @param flight_pos a pointer to the flight position to store the values
 *
 * @return true if a position was found or false on EOF
 */
bool igc_read_next_pos(int32_t igc_log_read_file, flight_pos_t *flight_pos)
{
	char line[80];
	static int16_t month, day, year;

	while(file_gets(line, sizeof(line), igc_log_read_file) != NULL)
	{
		if (strstr(line, "HFDTE") == line)
		{
			day   = atoi_n(line+5, 2);
			month = atoi_n(line+7, 2);
			year  = atoi_n(line+9, 2) + 2000;
		}
		if (line[0] == 'B')
		{
			int hour = atoi_n(line+1, 2);
			int min  = atoi_n(line+3, 2);
			int sec  = atoi_n(line+5, 2);

			//DBG("B timedate %02d.%02d.%04d %02d:%02d.%02d", day, month, year, hour, min, sec);

			flight_pos->timestamp = datetime_to_epoch(sec, min, hour, day, month, year);

			//DBG("B timestamp %ld", flight_pos->timestamp);
			flight_pos->lat = atoi_n(line+7, 2) * GNSS_MUL + atoi_n(line+9, 5) * (GNSS_MUL / 60000);
			if (line[14] == 'S') flight_pos->lat = -flight_pos->lat;
			flight_pos->lon = atoi_n(line+15, 3) * GNSS_MUL + atoi_n(line+18, 5) * (GNSS_MUL / 60000);
			if (line[24] == 'W') flight_pos->lon = -flight_pos->lon;

			flight_pos->baro_alt = atoi_n(line+25, 5);
			flight_pos->gnss_alt = atoi_n(line+30, 5);

			return true;
		}
	}
	return false;
}

/**
 * Read the flight statistics from a igc file. This is done by
 * parsing the IGC file opened with "fp".
 *
 * @param fp a file handle of the log file to parse
 * @param f_stat a pointer to the flight statistics to store the values
 */
void igc_read_flight_stats(int32_t fp, flight_stats_t *f_stat)
{
	flight_pos_t first_pos, last_pos, pos;
	int16_t raise;

	f_stat->max_climb = 0;
	f_stat->max_sink = 0;
	f_stat->min_alt = INT16_MAX;
	f_stat->max_alt = 0;
    f_stat->max_lat = INT32_MIN;
    f_stat->max_lon = INT32_MIN;
    f_stat->min_lat = INT32_MAX;
    f_stat->min_lon = INT32_MAX;

	igc_read_next_pos(fp, &first_pos);

	f_stat->start_time = first_pos.timestamp;

	last_pos = first_pos;
	while ( igc_read_next_pos(fp, &pos) )
	{
		f_stat->odo += geo_distance(last_pos.lat, last_pos.lon, pos.lat, pos.lon, false, NULL);

		raise = (pos.baro_alt - last_pos.baro_alt) * 100;   // meter to cm
		f_stat->max_climb = max(f_stat->max_climb, raise);
		f_stat->max_sink = min(f_stat->max_sink, raise);

		f_stat->max_alt = max(f_stat->max_alt, pos.baro_alt);
		f_stat->min_alt = min(f_stat->min_alt, pos.baro_alt);

        f_stat->min_lat = min(f_stat->min_lat, pos.lat);                                                                                                          
        f_stat->max_lat = max(f_stat->max_lat, pos.lat);                                                                                                          
        f_stat->min_lon = min(f_stat->min_lon, pos.lon);                                                                                                        
        f_stat->max_lon = max(f_stat->max_lon, pos.lon);                                                                                                        

		last_pos = pos;
	}

	f_stat->duration = last_pos.timestamp - f_stat->start_time;
}
