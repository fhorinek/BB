/*
 * flightbook_statistics.c
 *
 *  Created on: Oct 03, 2022
 *      Author: tilmann@bubecks.de
 *
 * This shows statistics of all saved flights, like total hours flown,
 * distance, number of flights in various time periods.
 *
 * The implementation first reads in all IGC files and stores the important data
 * in a struct stat_entry. Such a stat_entry_t sums up all flights of a specific
 * day.
 *
 * The user then decides, what data he wants to see by setting datatype_t,
 * e.g. DATATYPE_HOURS to see how many hours he flew or DATATYPE_NUM to see how
 * many flights he did.
 *
 * By setting period_t he decided, what time period should be shown, e.g. yearly or
 * monthly.
 *
 * Depending on the selection, the data will be summarized in summarize_data(), which
 * does the main job.
 */

// #define DEBUG_LEVEL	DEBUG_DBG
#include <gui/tasks/menu/flightbook/flightbook_flight_map.h>
#include <gui/tasks/menu/flightbook/flightbook_statistics.h>
#include <gui/tasks/menu/flightbook/flightbook_flight.h>
#include <gui/tasks/menu/flightbook/flightbook.h>
#include <gui/tasks/menu/settings.h>
#include "gui/gui_list.h"
#include "gui/map/tile.h"
#include "gui/map/map_obj.h"
#include "gui/map/map_thread.h"
#include "fc/fc.h"
#include "fc/logger/igc.h"
#include "drivers/tft/tft.h"
#include "drivers/rtc.h"
#include "etc/format.h"
#include "etc/geo_calc.h"
#include "gui/dialog.h"
#include "gui/tasks/filemanager.h"
#include "etc/epoch.h"

/** The cache entry to store flight data about a specific day. */
typedef struct stat_entry
{
    uint32_t timestamp;
    /* This is the same date as "timestamp" and converted for easier access. */
    uint8_t year_since_70, month, day;

    uint8_t start_num;   // number of flights on this day.
    uint16_t odo_100m;   // in 100m. Maximum is 6554km
    uint16_t minutes;
} stat_entry_t;

/** What time period should be shown. */
typedef enum period
{
    PERIOD_ALL_YEARS,             // Display a yearly summary for every year available
    PERIOD_MONTHES_IN_YEAR,       // Display a monthly summary for the current year
    PERIOD_DAYS_IN_MONTH,         // Display single days
    PERIOD_First = PERIOD_ALL_YEARS,
    PERIOD_Last = PERIOD_DAYS_IN_MONTH
} period_t;

/** What information should be shown. */
typedef enum datatype
{
    DATATYPE_HOURS,               // Number of hours flown.
    DATATYPE_NUM,                 // Number of flights
    DATATYPE_ODO,                 // Distance flown
    DATATYPE_First = DATATYPE_HOURS,
    DATATYPE_Last = DATATYPE_ODO
} datatype_t;

#define STATS_NUM 1000   // How many flights can we process (in PSRAM)

#define ROW_NUM 12       // The number of rows of data

#define BAR_X 55         // The x position of the bar
#define BAR_WIDTH 125    // the maximum width of the bar

REGISTER_TASK_ILS(flightbook_statistics,

/**
 * This stores the timestamp of the data to show.
 * It points to the newest data which is typically in the lowest row.
 */
uint64_t newest_timestamp;

/* This is the same date as "newest_timestamp" and converted for easier access. */
uint16_t year;
        uint8_t month;
        uint8_t day;

        period_t period;          // What period should be shown?
        datatype_t datatype;// What data should be shown?

        lv_obj_t *par;// parent lv_obj_t

        bool recalculate;// Set to "true" to trigger a recalculation of the data

        bool data_available;// Is the "stats" data available? Set to "true" after asynchronous reader finishes.
        stat_entry_t *stats;// pointer to flight stat_entry_t in psram

        int year_min, year_max;// The first and last year found in the "stats".

        lv_obj_t *title;// The title label

        lv_obj_t *labels_when[ROW_NUM];// A label describing the day/month/year

        lv_obj_t *bars[ROW_NUM];// The bar of each ROW
        lv_point_t points[ROW_NUM][2];// ... and its points

        lv_obj_t *labels_value[ROW_NUM];// The label showing the value of the bar as a text

        float values[ROW_NUM];// the values shown in each row

        lv_obj_t *label_total;// The label showing the total
        lv_obj_t *label_average;   // The average value
        lv_obj_t *label_period;// label explaining, what period will be next
        lv_obj_t *label_type;// label explaing, what datatype will be next
        );

void flightbook_statistics_options_cb(uint8_t a, void *none)
{
}

/**
 * Derive local->day/month/year from local->newest_timestamp.
 */
static void compute_YMD()
{
    uint8_t sec, min, hour, wday;
    datetime_from_epoch(local->newest_timestamp, &sec, &min, &hour, &local->day, &wday, &local->month, &local->year);
}

/**
 * Compute the next "newest_timestamp" depending on direction and time period.
 *
 * @param direction "+1" for "later" and "-1" for earlier.
 */
static void goto_next_timestamp(int direction)
{
    uint8_t sec, min, hour, day, wday, month;
    uint16_t year;

    switch (local->period)
    {
        case PERIOD_MONTHES_IN_YEAR:
            {
            datetime_from_epoch(local->newest_timestamp, &sec, &min, &hour, &day, &wday, &month, &year);
            year += direction;
            local->newest_timestamp = datetime_to_epoch(sec, min, hour, day, month, year);
            break;
        }
        case PERIOD_DAYS_IN_MONTH:
            {
            local->newest_timestamp += (int32_t) direction * ROW_NUM * 24 * 60 * 60;    // 7 days
            break;
        }
        case PERIOD_ALL_YEARS:
            break;
    }

    uint32_t min_timestamp = datetime_to_epoch(0, 0, 0, 1, 1, local->year_min);
    uint32_t max_timestamp = datetime_to_epoch(59, 59, 23, 31, 12, local->year_max);

    local->newest_timestamp = max(local->newest_timestamp, min_timestamp);
    local->newest_timestamp = min(local->newest_timestamp, max_timestamp);

    compute_YMD();
}

static void flightbook_statistics_cb(lv_obj_t *obj, lv_event_t event)
{
    uint32_t key = 0;

    switch (event)
    {
        case LV_EVENT_CANCEL:
            flightbook_open(false);
        break;
        case LV_EVENT_CLICKED:
            if (local->datatype == DATATYPE_Last)
                local->datatype = DATATYPE_First;
            else
                local->datatype++;
            local->recalculate = true;
        break;
        case LV_EVENT_KEY:
            key = *((uint32_t*) lv_event_get_data());
            switch (key)
            {
                case LV_KEY_HOME:
                    if (local->period == PERIOD_Last)
                        local->period = PERIOD_First;
                    else
                        local->period++;
                    local->recalculate = true;
                break;
                case LV_KEY_RIGHT:
                    goto_next_timestamp(1);
                    local->recalculate = true;
                break;
                case LV_KEY_LEFT:
                    goto_next_timestamp(-1);
                    local->recalculate = true;
                break;
            }
    }
}

/**
 * Read IGC files from the given directory.
 *
 * @param path the directory name to read from
 */
static void read_dir(char *path)
{
    flight_stats_t f_stat;
    REDDIRENT *entry;
    int i;

    REDDIR *dir = red_opendir(path);
    if (dir != NULL)
    {
        while ((entry = red_readdir(dir)))
        {
            //hide system files
            if (entry->d_name[0] == '.')
                continue;

            char full_path[PATH_LEN];
            sprintf(full_path, "%s/%s", path, entry->d_name);

            if (RED_S_ISDIR(entry->d_stat.st_mode))
            {
                read_dir(full_path);
            }
            else
            {
                gui_lock_acquire();
                dialog_progress_set_subtitle(entry->d_name);
                gui_lock_release();

                logger_read_flight_stats(full_path, &f_stat);
                if (f_stat.start_time != FS_NO_DATA)
                {
                    uint8_t sec, min, hour, day, wday, month;
                    uint16_t year;
                    datetime_from_epoch(f_stat.start_time, &sec, &min, &hour, &day, &wday, &month, &year);
                    year -= 1970;
                    for (i = 0; i < STATS_NUM; i++)
                    {
                        if ((local->stats[i].year_since_70 == year && local->stats[i].month == month && local->stats[i].day == day)
                                || local->stats[i].year_since_70 == 0)
                        {
                            local->year_min = min(local->year_min, year + 1970);
                            local->year_max = max(local->year_max, year + 1970);

                            local->stats[i].timestamp = f_stat.start_time;
                            local->stats[i].year_since_70 = year;
                            local->stats[i].month = month;
                            local->stats[i].day = day;

                            local->stats[i].odo_100m += f_stat.odo / 10000;   // cm to 100m
                            local->stats[i].minutes += f_stat.duration / 60;   // sec to min
                            local->stats[i].start_num++;

                            DBG("%d %s: odo_100m=%d minutes=%d start_num=%d", i, full_path, local->stats[i].odo_100m,local->stats[i].minutes, local->stats[i].start_num);
                            break;
                        }
                    }
                }
            }
        }
        red_closedir(dir);
    }
}

/**
 * Compute the index into local->values which should be used to
 * store the entry. This depends on local->period how values
 * is interpreted.
 *
 * @param entry the entry to process.
 *
 * @return the index of this entry into local->values[] or -1 if outside
 */
static int get_index(stat_entry_t *entry)
{
    int index = -1;
    uint64_t oldest_timestamp;

    switch (local->period)
    {
        case PERIOD_MONTHES_IN_YEAR:
            if (entry->year_since_70 + 1970 == local->year)
                // we store monthly values at the position of the month
                index = entry->month - 1;
        break;
        case PERIOD_DAYS_IN_MONTH:
            // Compute what day period we will show: from "oldest_timestamp" to "newest_timestamp":
            oldest_timestamp = local->newest_timestamp - (ROW_NUM * 24 * 60 * 60);
            if (oldest_timestamp < entry->timestamp && entry->timestamp < local->newest_timestamp)
                // Every day has its own local->values
                index = (entry->timestamp - oldest_timestamp) / (24 * 60 * 60);
        break;
        case PERIOD_ALL_YEARS:
            // Every year is in its own local->values.
            index = entry->year_since_70 + 1970 - local->year_min;
        break;
    }

    return index;
}

/**
 * Summarize the flight data depending on time period and datatype.
 * The result is put into local->values[] and bars and labels are
 * written and set up.
 */
static void summarize_data()
{
    int i;
    float max_value = 0;
    float total = 0;
    int total_flights = 0;
    float value;
    int index;
    char text[30], text2[30];

    // clear all values
    for (i = 0; i < ROW_NUM; i++)
        local->values[i] = 0;

    for (i = 0; i < STATS_NUM; i++)
    {
        index = get_index(&local->stats[i]);
        if (index >= 0 && index < ROW_NUM)
        {
            switch (local->datatype)
            {
                case DATATYPE_HOURS:
                    value = local->stats[i].minutes / 60.0;
                break;
                case DATATYPE_ODO:
                    value = local->stats[i].odo_100m / 10.0;
                break;
                case DATATYPE_NUM:
                    value = local->stats[i].start_num;
                break;
                default:
                    value = 0;
                break;
            }
            local->values[index] += value;
            max_value = max(max_value, local->values[index]);

            total_flights += local->stats[i].start_num;
        }
    }

    uint64_t oldest_timestamp, this_timestamp;

    for (i = 0; i < ROW_NUM; i++)
    {
        char label_t[10];
        float value;

        value = local->values[i];

        // [1] Create the label depending on the period
        switch (local->period)
        {
            case PERIOD_MONTHES_IN_YEAR:
                strcpy(label_t, month_names[i]);
            break;
            case PERIOD_DAYS_IN_MONTH:
                oldest_timestamp = local->newest_timestamp - ((ROW_NUM - 1) * 24 * 60 * 60);   // -1 because we want to include "today" as last day
                this_timestamp = oldest_timestamp + i * (24 * 60 * 60);
                uint8_t sec, min, hour, day, wday, month;
                uint16_t year;
                datetime_from_epoch(this_timestamp, &sec, &min, &hour, &day, &wday, &month, &year);
                format_date_DM(label_t, day, month);
            break;
            case PERIOD_ALL_YEARS:
                {
                uint16_t year = local->year_min + i;
                if (year <= local->year_max)
                    sprintf(label_t, "%d", year);
                else
                    strcpy(label_t, "");
                break;
            }
        }
        lv_label_set_text(local->labels_when[i], label_t);

        // [2] Depending on value, set correct bar length and bar label
        if (value <= 0.1)
        {
            lv_obj_set_hidden(local->bars[i], true);
            lv_obj_set_hidden(local->labels_value[i], true);
        }
        else
        {
            local->points[i][1].x = local->points[i][0].x + value * BAR_WIDTH / max_value;
            lv_line_set_points(local->bars[i], &local->points[i][0], 2);
            lv_obj_set_hidden(local->bars[i], false);
            lv_obj_set_x(local->labels_value[i], local->points[i][1].x + 10);
            if (local->datatype != DATATYPE_NUM)
                sprintf(text, "%.1f", value);
            else
                sprintf(text, "%.0f", value);

            total += value;
            lv_label_set_text(local->labels_value[i], text);
            lv_obj_set_hidden(local->labels_value[i], false);
        }
    }

    // [3] Show total
    switch (local->datatype)
    {
        case DATATYPE_ODO:
            strcpy(text, "Total: ");
            format_distance_with_units2(text + 7, total * 1000);
            strcpy(text2, "Average per flight: ");
            format_distance_with_units2(text2 + 20, total * 1000 / total_flights);
        break;
        case DATATYPE_HOURS:
            sprintf(text, "Total: %.1f hours", total);
            sprintf(text2, "Average per flight: %.1f hours", total / total_flights);
        break;
        case DATATYPE_NUM:
            sprintf(text, "Total: %.0f flights", total);
            text2[0] = 0;
        break;
    }
    lv_label_set_text(local->label_total, text);
    lv_label_set_text(local->label_average, text2);
}

void flightbook_statistics_load_task(void *param)
{
    int i;

    local->data_available = false;
    local->year_min = 3000;
    local->year_max = 0;

    local->stats = ps_malloc(sizeof(stat_entry_t) * STATS_NUM);
    for (i = 0; i < STATS_NUM; i++)
    {
        local->stats[i].year_since_70 = 0;
        local->stats[i].minutes = 0;
        local->stats[i].odo_100m = 0;
        local->stats[i].start_num = 0;
    }

    read_dir(PATH_LOGS_DIR);

    local->data_available = true;

    dialog_close();

    // Check, if we read too many flights.
    for (i = 0; i < STATS_NUM; i++)
    {
        if (local->stats[i].year_since_70 == 0)
            break;
    }

    if (i >= STATS_NUM)
    {
        dialog_show("Too Many Flight", "You have too many flights to handle in statistics. Please remove some.", dialog_confirm, flightbook_statistics_options_cb);
    }

    gui_low_priority(false);

    RedTaskUnregister();
    vTaskDelete(NULL);
}

void flightbook_statistics_load()
{
    dialog_show("Reading...", PATH_LOGS_DIR, dialog_progress, NULL);
    dialog_progress_spin();
    gui_low_priority(true);

    //create function that will process the data in separate task, so GUI won't became unresponsive
    xTaskCreate((TaskFunction_t) flightbook_statistics_load_task, "fb_stat_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);
}

lv_obj_t* flightbook_statistics_init(lv_obj_t *par)
{
    lv_obj_t *chart;
    int i;

    local->par = par;

    local->newest_timestamp = rtc_get_epoch();
    compute_YMD();

    local->period = PERIOD_MONTHES_IN_YEAR;
    local->datatype = DATATYPE_HOURS;
    local->recalculate = true;
    local->data_available = false;

    local->title = lv_label_create(par, NULL);
    lv_obj_set_style_local_text_font(local->title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_22);
    lv_obj_align(local->title, par, LV_ALIGN_IN_TOP_MID, 0, 10);
    lv_label_set_text(local->title, "");

    chart = lv_obj_create(par, NULL);
    lv_obj_set_size(chart, lv_obj_get_width(par), lv_obj_get_height(par) - 40);
    lv_obj_set_pos(chart, 0, 40);

    gui_set_dummy_event_cb(par, flightbook_statistics_cb);

    for (i = 0; i < ROW_NUM; i++)
    {
        lv_coord_t y = i * 20;
        local->labels_when[i] = lv_label_create(chart, NULL);
        lv_obj_set_pos(local->labels_when[i], 0, y);
        lv_label_set_text(local->labels_when[i], month_names[i]);

        local->points[i][0].x = BAR_X;
        local->points[i][0].y = y + 10;
        local->points[i][1].x = BAR_X;
        local->points[i][1].y = y + 10;
        local->bars[i] = lv_line_create(chart, NULL);
        lv_line_set_points(local->bars[i], &local->points[i][0], 2);
        lv_obj_set_style_local_line_color(local->bars[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);
        lv_obj_set_style_local_line_width(local->bars[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 10);
        lv_obj_set_hidden(local->bars[i], true);

        local->labels_value[i] = lv_label_create(chart, NULL);
        lv_obj_set_pos(local->labels_value[i], 200, y);
        lv_obj_set_hidden(local->labels_value[i], true);
    }

    local->label_total = lv_label_create(chart, NULL);
    lv_obj_set_pos(local->label_total, 0, (ROW_NUM + 1) * 20);
    lv_label_set_text(local->label_total, "");

    local->label_average = lv_label_create(chart, NULL);
    lv_obj_set_pos(local->label_average, 0, (ROW_NUM + 2) * 20);
    lv_label_set_text(local->label_average, "");

    local->label_type = lv_label_create(par, NULL);
    lv_obj_align(local->label_type, par, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_label_set_text(local->label_type, "");

    local->label_period = lv_label_create(par, NULL);
    lv_obj_align(local->label_period, par, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    lv_label_set_text(local->label_period, "");

    return chart;
}

void flightbook_statistics_loop()
{
    char period_title[32];
    char text[40];
    char title_text[64];

    if (!local->data_available)
        return;

    if (local->recalculate)
    {
        local->recalculate = false;

        summarize_data();

        switch (local->period)
        {
            case PERIOD_ALL_YEARS:
                strcpy(text, "Year");
                sprintf(period_title, "yearly");
            break;
            case PERIOD_MONTHES_IN_YEAR:
                strcpy(text, "Days");
                sprintf(period_title, "%d", local->year);
            break;
            case PERIOD_DAYS_IN_MONTH:
                strcpy(text, "Total");
                sprintf(period_title, "%d/%02d", local->year, local->month);
            break;
        }

        lv_label_set_text_fmt(local->label_period, LV_SYMBOL_REFRESH " %s", text);
        lv_obj_align(local->label_period, local->par, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

        switch (local->datatype)
        {
            case DATATYPE_HOURS:
                strcpy(text, "Flights");
                sprintf(title_text, "Hours %s", period_title);
            break;
            case DATATYPE_NUM:
                strcpy(text, "Distance");
                sprintf(title_text, "Flights %s", period_title);
            break;
            case DATATYPE_ODO:
                strcpy(text, "Time");
                sprintf(title_text, "Distance %s", period_title);
            break;
        }

        lv_label_set_text_fmt(local->label_type, LV_SYMBOL_REFRESH " %s", text);
        lv_obj_align(local->label_type, local->par, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

        if (local->period == PERIOD_ALL_YEARS)
            lv_label_set_text(local->title, title_text);
        else
            lv_label_set_text_fmt(local->title, LV_SYMBOL_LEFT " %s " LV_SYMBOL_RIGHT, title_text);

        lv_obj_align(local->title, local->par, LV_ALIGN_IN_TOP_MID, 0, 10);
    }
}

void flightbook_statistics_stop()
{
    ps_free(local->stats);
}
