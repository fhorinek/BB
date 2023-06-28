/*
 * logger.c
 *
 *  Created on: Jun 30, 2021
 *      Author: horinek
 */

// #define DEBUG_LEVEL	DBG_DEBUG
#include "logger.h"

#include <inttypes.h>

#include "fc/fc.h"
#include "config/config.h"

#include "igc.h"
#include "csv.h"
#include "kml.h"

#define STATS_VERSION		3
#define STATS_TOTAL_RECORDS	10

fc_logger_status_t logger_state()
{
    return (fc.logger.igc > fc.logger.csv ? fc.logger.igc : fc.logger.csv);
}

/**
 * Convert the flight statistics into a text format containing
 * multiple lines.
 *
 * @param f_stat the flight statistics to convert
 * @param buffer the buffer to hold the data [at least 300 bytes]
 */
static void flight_stats_to_text(flight_stats_t *f_stat, char *buffer)
{
    sprintf(buffer, " SKYBEAN-STATS: %u\n"
                    " SKYBEAN-START-UTC-s: %" PRIu32 "\n"
                    " SKYBEAN-TZ-OFFSET-s: %" PRId32 "\n"
                    " SKYBEAN-DURATION-s: %" PRIu32 "\n"
                    " SKYBEAN-ALT-MAX-m: %" PRId16 "\n"
                    " SKYBEAN-ALT-MIN-m: %" PRId16 "\n"
                    " SKYBEAN-CLIMB-MAX-cm: %" PRId16 "\n"
                    " SKYBEAN-SINK-MAX-cm: %" PRId16 "\n"
                    " SKYBEAN-ODO-m: %" PRIu32 "\n"
                    " SKYBEAN-BBOX: %" PRId32 " %" PRId32 " %" PRId32 " %" PRId32 "\n",
                    STATS_VERSION,
                    f_stat->start_time,
					f_stat->tz_offset,
                    f_stat->duration,
                    f_stat->max_alt,
                    f_stat->min_alt,
                    f_stat->max_climb,
                    f_stat->max_sink,
                    f_stat->odo / 100,  // cm in m
                    f_stat->min_lat, f_stat->max_lat, f_stat->min_lon, f_stat->max_lon);
}

/**
 * Write the given flight statistics into the log file by using
 * special keywords inside comments.
 *
 * @param f_stat the flight statistics to save
 */
void logger_write_flight_stats(flight_stats_t f_stat)
{
    char buffer[300];

    flight_stats_to_text(&f_stat, buffer);
    logger_comment(buffer);
}

/**
 * Read the flight statistics from a file where they are stored
 * with special comments.
 *
 * @param fp the file pointer to read from
 * @param f_stat a pointer to the flight statistics to save
 *
 * return false if not all parameters are found or stats are wrong version
 */
static bool read_stats_from_file(int32_t fp, flight_stats_t *f_stat)
{
    char line[80];
    char *p;

    uint8_t records = 0;

    while (1)
    {
        if (file_gets(line, sizeof(line), fp) == NULL)
            break;

        p = strstr(line, "SKYBEAN-STATS: ");
        if (p != NULL)
        {
            uint8_t stats_version = atoi(p + 15);
            if (stats_version != STATS_VERSION)
                break;

            records++;
            continue;
        }

        p = strstr(line, "SKYBEAN-START-UTC-s: ");
        if (p != NULL)
        {
            f_stat->start_time = atol(p + 21);
            records++;
            continue;
        }

        p = strstr(line, "SKYBEAN-TZ-OFFSET-s: ");
        if (p != NULL)
        {
            f_stat->tz_offset = atol(p + 21);
            records++;
            continue;
        }

        p = strstr(line, "SKYBEAN-DURATION-s: ");
        if (p != NULL)
        {
            f_stat->duration = atol(p + 20);
            records++;
            continue;
        }

        p = strstr(line, "SKYBEAN-ALT-MAX-m: ");
        if (p != NULL)
        {
            f_stat->max_alt = atoi(p + 19);
            records++;
            continue;
        }

        p = strstr(line, "SKYBEAN-ALT-MIN-m: ");
        if (p != NULL)
        {
            f_stat->min_alt = atoi(p + 19);
            records++;
            continue;
        }

        p = strstr(line, "SKYBEAN-CLIMB-MAX-cm: ");
        if (p != NULL)
        {
            f_stat->max_climb = atoi(p + 22);
            records++;
            continue;
        }

        p = strstr(line, "SKYBEAN-SINK-MAX-cm: ");
        if (p != NULL)
        {
            f_stat->max_sink = atoi(p + 21);
            records++;
            continue;
        }

        p = strstr(line, "SKYBEAN-ODO-m: ");
        if (p != NULL)
        {
            f_stat->odo = atol(p + 15) * 100;   // meter in cm
            records++;
            continue;
        }

        p = strstr(line, "SKYBEAN-BBOX: ");
        if (p != NULL)
        {
            char *saveptr;
            p = strtok_r(p + 14, " ", &saveptr);
            f_stat->min_lat = atol(p);
            p = strtok_r(NULL, " ", &saveptr);
            f_stat->max_lat = atol(p);
            p = strtok_r(NULL, " ", &saveptr);
            f_stat->min_lon = atol(p);
            p = strtok_r(NULL, " ", &saveptr);
            f_stat->max_lon = atol(p);
            records++;

            continue;
        }
    }

    return records == STATS_TOTAL_RECORDS;
}

/**
 * Read the flight statistics from a log file. This is either done by
 * searching for the keywords and values or by parsing the IGC file,
 * if no keywords are found (for old IGC logs).
 *
 * @param filename the filename of the log file to parse
 * @param f_stat a pointer to the flight statistics to store the values
 */
void logger_read_flight_stats(const char *filename, flight_stats_t *f_stat)
{
    int32_t fp, fp_cache;
    char path[PATH_LEN];
    char *p;

    // Set defaults, if nothing could be found in the file:
    f_stat->start_time = FS_NO_DATA;
    f_stat->tz_offset = FS_NO_DATA;
    f_stat->duration = FS_NO_DATA;
    f_stat->max_alt = FS_NO_DATA;
    f_stat->min_alt = FS_NO_DATA;
    f_stat->max_climb = FS_NO_DATA;
    f_stat->max_sink = FS_NO_DATA;
    f_stat->odo = FS_NO_DATA;

    fp = red_open(filename, RED_O_RDONLY);
    if (fp < 0)
        return;

    red_lseek(fp, -512, RED_SEEK_END);    	// Read from the end of the file
    bool stats_valid = read_stats_from_file(fp, f_stat);

    if (!stats_valid)
    {
        // Fallback: this is an old file without comments, so read data out of files
        sprintf(path, PATH_LOG_CACHE_DIR "/%s", filename + 5);

        // make directory and its parents:
        p = strrchr(path, '/');
        *p = 0;
        red_mkdirs(path);
        *p = '/';

        bool cache_valid = false;

        fp_cache = red_open(path, RED_O_RDONLY);
        if (fp_cache >= 0)
        {
            // There is already a cache file for flight statistics
            cache_valid = read_stats_from_file(fp_cache, f_stat);
            red_close(fp_cache);

            if (!cache_valid)
            {
                red_unlink(path);
            }
        }

        if (!cache_valid)
        {
            // no cache file, so parse IGC directly...
            red_lseek(fp, 0, RED_SEEK_SET);
            igc_read_flight_stats(fp, f_stat);

            // ... and store values in the cache file for next reads
            fp_cache = red_open(path, RED_O_WRONLY | RED_O_CREAT | RED_O_TRUNC);
            if (fp_cache >= 0)
            {
                char buffer[300];

                flight_stats_to_text(f_stat, buffer);
                red_write(fp_cache, buffer, strlen(buffer));
                red_close(fp_cache);
            }
        }
    }
    red_close(fp);
}

void logger_init()
{
    igc_init();
    csv_init();
    kml_init();
}

void logger_start()
{
    if (config_get_bool(&profile.flight.logger.igc))
        igc_start();

    if (config_get_bool(&profile.flight.logger.csv))
        csv_start();

    if (config_get_bool(&profile.flight.logger.kml))
        kml_start();
}

/**
 * printf-like function to send output to the GPS log.
 * The resulting string may contain "\n" which will
 * result in multiple lines to be printed into the log.
 *
 * \param format a printf-like format string 
 **/
void logger_comment(const char *format, ...)
{
    va_list arp;
    char text[300];
    char *saveptr;
    char *pch;

    va_start(arp, format);
    vsnprintf(text, sizeof(text), format, arp);
    va_end(arp);

    // If text contains multiple lines (delimited by "\n"), then break them
    // into multiple calls to igc_comment (whatever).
    pch = strtok_r(text, "\n", &saveptr);
    while (pch != NULL)
    {
        igc_comment(pch);
        kml_comment(pch);
        // Add additional loggers here, if available.
        pch = strtok_r(NULL, "\n", &saveptr);
    }
}

void logger_stop()
{
    igc_stop();
    csv_stop();
    kml_stop();
}
