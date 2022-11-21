/*
 * airspace.cpp
 *
 *  Created on: Mar 14, 2022
 *      Author: horinek
 */
//#define DEBUG_LEVEL	DBG_DEBUG

#include "airspace.h"
#include "fc/fc.h"
#include "etc/geo_calc.h"
#include "gui/dialog.h"
#include "gui/statusbar.h"
#include "gui/tasks/filemanager.h"

typedef struct
{
    uint32_t cache_version;
    REDSTAT orig_file;
    uint32_t number_of_records;
    uint32_t index_size;
    uint32_t data_size;
    gnss_bbox_t bbox;
} airspace_header_t;

#define AIRSPACE_MAX_POINTS     2048
#define AIRSPACE_MAX_NAME_LEN   128
#define AIRSPACE_CACHE_VERSION  14

#define AIRSPACE_INDEX_ALLOC    1024
#define AIRSPACE_DATA_ALLOC    1024 * 1024


void airspace_init_buffer()
{
    fc.airspaces.index = ps_malloc(AIRSPACE_INDEX_ALLOC * sizeof(airspace_record_t));
    fc.airspaces.data = ps_malloc(AIRSPACE_DATA_ALLOC);
}


void airspace_create_lock()
{
    fc.airspaces.lock = osMutexNew(NULL);
    osMutexAcquire(fc.airspaces.lock, WAIT_INF);
    vQueueAddToRegistry(gui.lock, "airspace.lock");
    osMutexRelease(fc.airspaces.lock);
}

bool airspace_point(char * line, int32_t * lon, int32_t * lat)
{
    unsigned int lat_deg, lat_min, lat_sec, lat_dsec;
    unsigned int lon_deg, lon_min, lon_sec, lon_dsec;
    char lat_c, lon_c;

    uint8_t filled = sscanf(line, "%02u:%02u:%02u.%02u %c %03u:%02u:%02u.%02u %c",
                        &lat_deg, &lat_min, &lat_sec, &lat_dsec, &lat_c,
                        &lon_deg, &lon_min, &lon_sec, &lon_dsec, &lon_c);

    if (filled != 10)
    {
        lat_dsec = 0;
        lon_dsec = 0;
        filled = sscanf(line, "%02u:%02u:%02u %c %03u:%02u:%02u %c",
                        &lat_deg, &lat_min, &lat_sec, &lat_c,
                        &lon_deg, &lon_min, &lon_sec, &lon_c);

        if (filled != 8)
        {
            WARN("unable to read geo point %s", line);
            return false;
        }
    }

    *lat = lat_deg * GNSS_MUL + lat_min * (GNSS_MUL / 60) + lat_sec * (GNSS_MUL / 3600) + lat_dsec * (GNSS_MUL / 360000);
    if (lat_c == 'S') *lat *= -1;
    *lon = lon_deg * GNSS_MUL + lon_min * (GNSS_MUL / 60) + lon_sec * (GNSS_MUL / 3600) + lon_dsec * (GNSS_MUL / 360000);
    if (lon_c == 'W') *lon *= -1;

    if (abs(*lat) > 90 * GNSS_MUL || abs(*lon) > 180 * GNSS_MUL)
    {
        WARN("unable to read geo point %s", line);
        return false;
    }

    return true;
}

uint16_t airspace_alt(char * line, bool * gnd)
{
    bool fl = false;

    if (strstr(line, "GND") != NULL)
        *gnd = true;

    if (strncmp("FL", line, 2) == 0)
    {
        line += 2;
        fl = true;
    }

    uint16_t value = atoi_c(line);

    if (fl)
    {
        value = (value * 100) / FC_METER_TO_FEET;
        *gnd = false;
    }

    if (strstr(line, "ft") != NULL)
        value /= FC_METER_TO_FEET;

    if (strstr(line, "AMSL") != NULL)
        *gnd = false;

    return value;
}

//calculate bounding box
//write airspace
bool airspace_finalise(airspace_header_t * ah, airspace_record_t * as, gnss_pos_t * points, int index, int data)
{
	if (as->number_of_points > 0)
	{
		as->bbox.latitude1 = -90 * GNSS_MUL;
		as->bbox.latitude2 = +90 * GNSS_MUL;
		as->bbox.longitude1 = +180 * GNSS_MUL;
		as->bbox.longitude2 = -180 * GNSS_MUL;

		for (uint32_t i = 0; i < as->number_of_points; i++)
		{
			as->bbox.latitude1 = max(as->bbox.latitude1, points[i].latitude);
			as->bbox.latitude2 = min(as->bbox.latitude2, points[i].latitude);
			as->bbox.longitude1 = min(as->bbox.longitude1, points[i].longitude);
			as->bbox.longitude2 = max(as->bbox.longitude2, points[i].longitude);
		}

        //store points
        as->points.pos = ah->data_size;
        red_write(data, points, sizeof(gnss_pos_t) * as->number_of_points);
        ah->data_size += sizeof(gnss_pos_t) * as->number_of_points;

        //store airspace name
        uint8_t name_len = min(strlen(as->name.ptr) + 1, AIRSPACE_MAX_NAME_LEN);
        uint8_t rounded_len = ROUND4(name_len);
        red_write(data, as->name.ptr, rounded_len);
        as->name.len = name_len;
        ah->data_size += rounded_len;

        //store record
        red_write(index, as, sizeof(airspace_record_t));
        ah->index_size += sizeof(airspace_record_t);

        //update header
        ah->bbox.latitude1 = max(as->bbox.latitude1, ah->bbox.latitude1);
        ah->bbox.latitude2 = min(as->bbox.latitude2, ah->bbox.latitude2);
        ah->bbox.longitude1 = min(as->bbox.longitude1, ah->bbox.longitude1);
        ah->bbox.longitude2 = max(as->bbox.longitude2, ah->bbox.longitude2);
        ah->number_of_records++;;
	}
}

void airspace_add_point(airspace_record_t * as, gnss_pos_t * points, int32_t lon, int32_t lat)
{
    if (as->number_of_points >= AIRSPACE_MAX_POINTS)
    {
        WARN("aisrpace %s, max points reached", as->name);
        return;
    }

	points[as->number_of_points].latitude = lat;
	points[as->number_of_points].longitude = lon;

	as->number_of_points++;
}


//read openair file and store binary data to cache
bool airspace_parse(char * name, bool use_dialog)
{
    char path[PATH_LEN];
    snprintf(path, sizeof(path), "%s/%s", PATH_AIRSPACE_DIR, name);

    INFO("airspace_load %s", path);

    int f = red_open(path, RED_O_RDONLY);

    if (f > 0)
    {
        //reset header
        airspace_header_t ah;
        ah.bbox.latitude1 = -90 * GNSS_MUL;
        ah.bbox.latitude2 = +90 * GNSS_MUL;
        ah.bbox.longitude1 = +180 * GNSS_MUL;
        ah.bbox.longitude2 = -180 * GNSS_MUL;
        ah.cache_version = AIRSPACE_CACHE_VERSION;
        ah.data_size = 0;
        ah.index_size = 0;
        ah.number_of_records = 0;
        red_fstat(f, &ah.orig_file);

        //create dir
        red_mkdir(PATH_AIRSPACE_CACHE_DIR);

        //open files
        snprintf(path, sizeof(path), "%s/%s.index", PATH_AIRSPACE_CACHE_DIR, name);
        int index = red_open(path, RED_O_WRONLY | RED_O_CREAT);
        red_write(index, &ah, sizeof(airspace_header_t));
        ah.index_size += sizeof(airspace_header_t);

        snprintf(path, sizeof(path), "%s/%s.data", PATH_AIRSPACE_CACHE_DIR, name);
        int data = red_open(path, RED_O_WRONLY | RED_O_CREAT);

        //allocate memory for parser
        airspace_record_t as = {0};

        gnss_pos_t * points = tmalloc(sizeof(gnss_pos_t) * AIRSPACE_MAX_POINTS);
        char as_name[AIRSPACE_MAX_NAME_LEN];
        as.name.ptr = as_name;

        //handle gui
        lv_obj_t * msg_ptr = NULL;

        if (!use_dialog)
        {
            msg_ptr = statusbar_msg_add(STATUSBAR_MSG_PROGRESS, "Parsing airspaces");
        }

    	uint32_t size = file_size(f);
    	uint32_t pos = 0;
    	uint8_t gui_skip = 0;

    	//init parser
    	char mem_line[128];
        char * line;

        gnss_pos_t center = {0};
        bool clockwise = true;

        while (true)
        {
            if ((line = red_gets(mem_line, sizeof(mem_line), f)) == NULL)
                break;

            if (line == GETS_CORRUPTED)
            {
                statusbar_msg_add(STATUSBAR_MSG_ERROR, "Airspace file corrupted");
                break;
            }

            pos += strlen(line);

            if (gui_skip == 0)
            {
                if (use_dialog)
                {
                    dialog_progress_set_progress((pos * 100) / size);
                }
                else
                {
                    statusbar_msg_update_progress(msg_ptr, (pos * 100) / size);
                }
            }
            gui_skip = (gui_skip + 1) % 20;

            DBG("AIR: %s", line);

            if (line[0] == '*' || line[0] == '\r' || strlen(line) == 0)
                continue;

            //airspace class
            if (strncmp("AC ", line, 3) == 0)
            {
                //write airspace
                airspace_finalise(&ah, &as, points, index, data);

                //reset airspace
                as.name.ptr = as_name;
                as.name.ptr[0] = 0;
                as.number_of_points = 0;
                as.pen_width = 0;
                as.pen.full = 0;
                as.brush.full = 0;
                as.floor = 0;
                as.floor_gnd = true;
                as.ceil = 0;
                as.ceil_gnd = true;

                clockwise = true;

                line += 3;
                if (strncmp(line, "GP", 2) == 0)
                    as.airspace_class = ac_glider_prohibited;
                else if (strncmp(line, "CTR", 3) == 0)
                    as.airspace_class = ac_ctr;
                else if (strncmp(line, "TMZ", 3) == 0)
                    as.airspace_class = ac_tmz;
                else if (strncmp(line, "RMZ", 3) == 0)
                    as.airspace_class = ac_rmz;
                else if (*line == 'R')
                     as.airspace_class = ac_restricted;
                else if (*line == 'Q')
                    as.airspace_class = ac_danger;
                else if (*line == 'P')
                    as.airspace_class = ac_prohibited;
                else if (*line == 'A')
                    as.airspace_class = ac_class_A;
                else if (*line == 'B')
                    as.airspace_class = ac_class_B;
                else if (*line == 'C')
                    as.airspace_class = ac_class_C;
                else if (*line == 'D')
                    as.airspace_class = ac_class_D;
                else if (*line == 'E')
                    as.airspace_class = ac_class_E;
                else if (*line == 'F')
                    as.airspace_class = ac_class_F;
                else if (*line == 'G')
                    as.airspace_class = ac_class_G;
                else if (*line == 'W')
                    as.airspace_class = ac_wave_window;
               else
                    as.airspace_class = ac_undefined;

                continue;
            }
            //airspace name
            else if (strncmp("AN ", line, 3) == 0)
            {
                line += 3;

                uint8_t len = strlen_noend(line);
                len = min(AIRSPACE_MAX_NAME_LEN, len + 1);
                strncpy(as.name.ptr, line, len);
                as.name.ptr[len - 1] = 0;
                continue;
            }
            //Airspace ceiling
            else if (strncmp("AH ", line, 3) == 0)
            {
                line += 3;

                as.ceil = airspace_alt(line, &as.ceil_gnd);
            }
            //Airspace floor
            else if (strncmp("AL ", line, 3) == 0)
            {
                line += 3;

                as.floor = airspace_alt(line, &as.floor_gnd);
            }
            //Pen
            else if (strncmp("SP ", line, 3) == 0)
            {
                line += 3;

                //gui_skip style
                line = find_comma(line);
                //pen width
                as.pen_width = atoi_c(line) & PEN_WIDTH_MASK;
                line = find_comma(line);
                //pen red
                uint8_t red = atoi_c(line);
                line = find_comma(line);
                //pen green
                uint8_t green = atoi_c(line);
                line = find_comma(line);
                //pen blue
                uint8_t blue = atoi_c(line);

                as.pen = LV_COLOR_MAKE(red, green, blue);
            }
            //Brush
            else if (strncmp("SB ", line, 3) == 0)
            {
                line += 3;

                if (*line == '-') //-1,-1,-1
                {
                    as.pen_width |= BRUSH_TRANSPARENT_FLAG;
                }
                else
                {
					//brush red
					uint8_t red = atoi_c(line);
					line = find_comma(line);
					//brush green
					uint8_t green = atoi_c(line);
					line = find_comma(line);
					//brush blue
					uint8_t blue = atoi_c(line);

					as.brush = LV_COLOR_MAKE(red, green, blue);
                }
            }
            //polygon point
            else if (strncmp("DP ", line, 3) == 0)
            {
                line += 3;

                int32_t lon, lat;

                if (airspace_point(line, &lon, &lat))
                {
                	airspace_add_point(&as, points, lon, lat);
                }
            }
            //center point
            else if (strncmp("V X=", line, 4) == 0)
            {
                line += 4;

                int32_t lon, lat;

                if (airspace_point(line, &lon, &lat))
                {
                	center.latitude = lat;
                	center.longitude = lon;
                }
            }
            //circle direction
			else if (strncmp("V D=", line, 4) == 0)
			{
				line += 4;
				if (*line == '-')
					clockwise = false;
				if (*line == '+')
					clockwise = true;
			}
            //draw arc
            else if (strncmp("DA ", line, 3) == 0)
            {
                line += 3;

                uint16_t radius = atoi_c(line);
                float distance_km = radius * FC_NM_TO_KM;
                line = find_comma(line);
                int16_t start = atoi_c(line);
                int16_t end = atoi_c(line);

                int16_t dir = clockwise ? +5 : -5;

                bool done = false;
                //DBG("start %d, end %d, dir %d", start, end, dir);
                for (int16_t angle = start; !done;)
                {
                	int32_t lon, lat;

                	//DBG("angle %d", angle);

                	if (clockwise)
                	{
                		if (angle >= end)
                		{
                			angle = end;
                			done = true;
                		}
                	}
                	else
                	{
                		if (angle <= end)
                		{
                			angle = end;
                			done = true;
                		}
                	}

                    if (angle > 360)
                    {
                        angle %= 360;
                    }

                	geo_destination(center.latitude, center.longitude, angle, distance_km, &lat, &lon);
                	airspace_add_point(&as, points, lon, lat);

                    angle += dir;
                }
            }
            //draw arc2
            else if (strncmp("DB ", line, 3) == 0)
            {
                line += 3;

                int32_t lon1, lat1, lon2, lat2;

                if (!airspace_point(line, &lon1, &lat1))
                	continue;

                line = find_comma(line);

                if (!airspace_point(line, &lon2, &lat2))
                	continue;

                int16_t start;
                int16_t end;

                float distance_km = geo_distance(center.latitude, center.longitude, lat1, lon1, config_get_select(&config.units.earth_model) == EARTH_FAI, &start) / 100000.0;
                geo_distance(center.latitude, center.longitude, lat2, lon2, config_get_select(&config.units.earth_model), &end);

                int16_t dir = clockwise ? +5 : -5;

                if (clockwise)
                {
                    while (start > end)
                        end += 360;
                }
                else
                {
                    while (start < end)
                        start += 360;
                }

                bool done = false;
//                DBG("start %d, end %d, dir %d", start, end, dir);
                for (int16_t angle = start; !done; )
                {
                	int32_t lon, lat;

                	if (clockwise)
                	{
                		if (angle >= end)
                		{
                			angle = end;
                			done = true;
                		}
                	}
                	else
                	{
                		if (angle <= end)
                		{
                			angle = end;
                			done = true;
                		}
                	}

                	geo_destination(center.latitude, center.longitude, angle, distance_km, &lat, &lon);
                	airspace_add_point(&as, points, lon, lat);

                    angle += dir;
                }
            }
            //draw circle
            else if (strncmp("DC ", line, 3) == 0)
            {
                line += 3;

                float radius = atoi_f(line);
                float distance_km = radius * FC_NM_TO_KM;

                for (uint16_t angle = 0; angle < 360; angle += 5)
                {
                	int32_t lon, lat;

                	geo_destination(center.latitude, center.longitude, angle, distance_km, &lat, &lon);
                	airspace_add_point(&as, points, lon, lat);
                }
            }

        }

        //end of file
		//finalize last airspace here
        airspace_finalise(&ah, &as, points, index, data);

        red_lseek(index, 0, RED_SEEK_SET);
        red_write(index, &ah, sizeof(ah));

		red_close(f);
		red_close(index);
		red_close(data);


		if (msg_ptr != NULL)
		{
		    statusbar_msg_close(msg_ptr);
		}

		tfree(points);
		return true;
    }
    else
    {
        ERR("Unable to open %s, err = %u", path, f);
    }

    return false;
}


void airspace_reload_parallel_task(void * param)
{
    osMutexAcquire(fc.airspaces.lock, WAIT_INF);



    osMutexRelease(fc.airspaces.lock);

    RedTaskUnregister();
    vTaskDelete(NULL);
}

bool airspace_open_cache(char * name, airspace_header_t * ah, int32_t * findex, int32_t * fdata)
{
    char path[PATH_LEN];
    snprintf(path, sizeof(path), "%s/%s", PATH_AIRSPACE_DIR, name);

    int f = red_open(path, RED_O_RDONLY);

    if (f > 0)
    {
        REDSTAT fstat;
        red_fstat(f, &fstat);
        red_close(f);

        //open files
        snprintf(path, sizeof(path), "%s/%s.index", PATH_AIRSPACE_CACHE_DIR, name);
        int index = red_open(path, RED_O_RDONLY);
        if (index < 0)
        {
            return false;
        }

        snprintf(path, sizeof(path), "%s/%s.data", PATH_AIRSPACE_CACHE_DIR, name);
        int data = red_open(path, RED_O_RDONLY);
        if (data < 0)
        {
            red_close(index);
            return false;
        }

        red_read(index, ah, sizeof(airspace_header_t));

        bool valid = true;

        if (ah->cache_version != AIRSPACE_CACHE_VERSION)
            valid = false;

        if (memcmp(&ah->orig_file, &fstat, sizeof(REDSTAT)) != 0)
            valid = false;

        if (ah->index_size != file_size(index))
            valid = false;

        if (ah->data_size != file_size(data))
            valid = false;


        if (valid)
        {
            *findex = index;
            *fdata = data;

            return true;
        }
        else
        {
            red_close(index);
            red_close(data);

            return false;
        }
    }

    return false;
}


bool airspace_load(char * name, bool use_dialog)
{
    int32_t index, data;
    airspace_header_t ah;

    bool res = airspace_open_cache(name, &ah, &index, &data);

    if (!res)
    {
        res = airspace_parse(name, use_dialog);
        if (res)
        {
            res = airspace_open_cache(name, &ah, &index, &data);
        }
    }

    if (!res)
    {
        return false;
    }


    int32_t c_lon, c_lat;

    c_lon = gui.map.lon;
    c_lat = gui.map.lat;

    int32_t step_x;
    int32_t step_y;
    geo_get_steps(c_lat, 0xFF, &step_x, &step_y);

    //get bbox
    uint32_t map_w = MAP_W * step_x * 2;
    uint32_t map_h = MAP_H * step_y * 2;
    int32_t lon1 = c_lon - map_w / 2;
    int32_t lon2 = c_lon + map_w / 2;
    int32_t lat1 = c_lat + map_h / 2;
    int32_t lat2 = c_lat - map_h / 2;

    uint32_t record_index = 0;
    uint32_t data_index = 0;

    fc.airspaces.number_loaded = 0;
    fc.airspaces.number_in_file = ah.number_of_records;

    for (uint32_t i = 0; i < ah.number_of_records; i++)
    {
        airspace_record_t as;
        int32_t rd = red_read(index, &as, sizeof(airspace_record_t));

        if (rd != sizeof(airspace_record_t))
            break;

        //not working correctly?
        //check memeory limits index + data

        if (lon1 > as.bbox.longitude2 || lon2 < as.bbox.longitude1)
            continue;

        if (lat2 > as.bbox.latitude1 || lat1 < as.bbox.latitude2)
            continue;

        memcpy(&fc.airspaces.index[record_index], &as, sizeof(airspace_record_t));

        red_lseek(data, as.points.pos, RED_SEEK_SET);
        red_read(data, fc.airspaces.data + data_index, sizeof(gnss_pos_t) * as.number_of_points);
        fc.airspaces.index[record_index].points.ptr = fc.airspaces.data + data_index;
        data_index += sizeof(gnss_pos_t) * as.number_of_points;

        uint8_t name_len = ROUND4((uint32_t)as.name.len);
        red_read(data, fc.airspaces.data + data_index, name_len);
        fc.airspaces.index[record_index].name.ptr = fc.airspaces.data + data_index;
        fc.airspaces.index[record_index].name.ptr[name_len] = 0;

        data_index += name_len;

        record_index++;
        fc.airspaces.number_loaded++;
    }

    red_close(index);
    red_close(data);

    return true;
}

void airspace_unload()
{
    fc.airspaces.valid = false;
    fc.airspaces.number_in_file = 0;
    fc.airspaces.number_loaded = 0;
}

void airspace_reload_parallel()
{
    xTaskCreate((TaskFunction_t)airspace_reload_parallel_task, "as_load_parallel_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);
}


