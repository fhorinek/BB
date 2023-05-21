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

// use airspace_class_t as an index to get the name of the airspace type
char *airspace_class_names[] = {
		  "Restricted",
		  "Danger",
		  "Prohibited",
		  "A",
		   "B",
		    "C",
		    "D",
		    "E",
		    "F",
		    "G",
		    "Glider prohibited",
		    "CTR",
		    "TMZ",
		    "RMZ",
		    "Wave Window",
		    "Undef",
};

// use airspace_class_t as an index to get the color of the airspace type
lv_color_t airspace_class_brushes[] = {
	LV_COLOR_ORANGE,	 // "Restricted",
	LV_COLOR_ORANGE,	 // "Danger",
	LV_COLOR_RED,	     // "Prohibited",
	LV_COLOR_RED,	     // "A",
	LV_COLOR_RED,	     // "B",
	LV_COLOR_RED,	     // "C",
	LV_COLOR_RED,	     // "D",
	LV_COLOR_YELLOW,	 // "E",
	LV_COLOR_GREEN,	     // "F",
	LV_COLOR_GREEN,	     // "G",
	LV_COLOR_RED,	     // "Glider prohibited",
	LV_COLOR_RED,	     // "CTR",
	LV_COLOR_RED,	     // "TMZ",
	LV_COLOR_ORANGE,	 // "RMZ",
	LV_COLOR_WHITE,	     // "Wave Window",
	LV_COLOR_WHITE	     // "Undef",
};

/**
 * Return the name of a given airspace as a string.
 *
 * @param class the airspace class
 *
 * @return pointer to a statically allocated character string giving the name of the airspace class.
 */
char *airspace_class_name(airspace_class_t class)
{
	return airspace_class_names[class];
}

void airspace_init_buffer()
{
    fc.airspaces.lock = osMutexNew(NULL);
    vQueueAddToRegistry(fc.airspaces.lock, "airspace.lock");

    fc.airspaces.index = ps_malloc(AIRSPACE_INDEX_ALLOC * sizeof(airspace_record_t));
    fc.airspaces.data = ps_malloc(AIRSPACE_DATA_ALLOC);
    osMutexRelease(fc.airspaces.lock);
}

static bool airspace_point(char * line, int32_t * lon, int32_t * lat)
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

static uint16_t airspace_alt(char * line, bool * gnd)
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
static bool airspace_finalise(airspace_header_t * ah, airspace_record_t * as, gnss_pos_t * points, int index, int data)
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

bool airspace_is_previous_point(airspace_record_t * as, gnss_pos_t * points, int32_t lon, int32_t lat)
{
    return (as->number_of_points > 0
    		&& points[as->number_of_points-1].latitude == lat && points[as->number_of_points-1].longitude == lon);
}

void airspace_add_point(airspace_record_t * as, gnss_pos_t * points, int32_t lon, int32_t lat)
{
	// Do not enter the same point again.
    if (airspace_is_previous_point(as, points, lon, lat))
    	return;

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
static bool airspace_parse(char * name, bool use_dialog)
{
    char path[PATH_LEN];
    snprintf(path, sizeof(path), "%s/%s", PATH_AIRSPACE_DIR, name);

    INFO("airspace_load %s", path);

    int f = red_open(path, RED_O_RDONLY);

    bool error_load = false;

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
        int index = red_open(path, RED_O_WRONLY | RED_O_CREAT | RED_O_TRUNC);
        red_write(index, &ah, sizeof(airspace_header_t));
        ah.index_size += sizeof(airspace_header_t);

        snprintf(path, sizeof(path), "%s/%s.data", PATH_AIRSPACE_CACHE_DIR, name);
        int data = red_open(path, RED_O_WRONLY | RED_O_CREAT | RED_O_TRUNC);

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
                error_load = true;
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

                // Set standard color for the given airspace class. Maybe overridden by SP or SB later
                as.brush = airspace_class_brushes[as.airspace_class];
                as.pen = airspace_class_brushes[as.airspace_class];
                as.pen_width = 1;
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

                airspace_add_point(&as, points, lon1, lat1);

                // As the first and last point is given in DB, we do not use arc arithmetic to compute that point.
                // The last point is added explicitly at the end to avoid rounding errors.
            	start += dir;
                end -= dir;

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
            	airspace_add_point(&as, points, lon2, lat2);
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

		if (error_load || ah.number_of_records == 0)
		{
	        snprintf(path, sizeof(path), PATH_AIRSPACE_CACHE_DIR "/%s.index", name);
	        red_unlink(path);

	        snprintf(path, sizeof(path), PATH_AIRSPACE_CACHE_DIR "/%s.data", name);
	        red_unlink(path);

	        return false;
		}

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

        if (ah->number_of_records == 0)
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

static void airspace_force_redraw()
{
    //force redraw map
    for (uint8_t i = 0; i < MAP_CHUNKS; i++)
    {
        if (gui.map.chunks[i].ready
                && gui.map.chunks[i].airspace
                && !gui.map.chunks[i].airspace_nothing_drawn)
        {
            gui.map.chunks[i].ready = false;
        }
    }
}

void airspace_unload_unlocked()
{
    fc.airspaces.valid = false;
    fc.airspaces.number_in_file = 0;
    fc.airspaces.number_loaded = 0;
    fc.airspaces.data_used = 0;

    airspace_force_redraw();
}

bool airspace_load(char * name, bool use_dialog)
{
    int32_t index, data;
    airspace_header_t ah;

    airspace_unload_unlocked();

    bool res = false;

    if (strlen(name) > 0)
    {
        res = airspace_open_cache(name, &ah, &index, &data);

        if (!res)
        {
            res = airspace_parse(name, use_dialog);
            if (res)
            {
                res = airspace_open_cache(name, &ah, &index, &data);
            }
        }
    }

    if (!res)
    {
        airspace_force_redraw();
        return false;
    }


    int32_t c_lon, c_lat;

    c_lon = gui.map.lon;
    c_lat = gui.map.lat;

    int32_t step_x;
    int32_t step_y;
    geo_get_steps(c_lat, 0xFF, &step_x, &step_y);

    //get bbox
    uint32_t map_w = LV_HOR_RES * step_x;
    uint32_t map_h = LV_VER_RES * step_y;
    int32_t lon1 = c_lon - map_w / 2;
    int32_t lon2 = c_lon + map_w / 2;
    int32_t lat1 = c_lat + map_h / 2;
    int32_t lat2 = c_lat - map_h / 2;

    uint32_t record_index = 0;
    uint32_t data_index = 0;

    fc.airspaces.number_loaded = 0;
    fc.airspaces.number_in_file = ah.number_of_records;

    uint8_t filter_len = (&profile.airspace.display.undefined - &profile.airspace.display.restricted) + 1;
    bool filter[filter_len];
    for (uint8_t i = 0; i < filter_len ; i++)
    {
        cfg_entry_t * e = &profile.airspace.display.restricted + i;

        filter[i] = !e->value.b;
    }

    uint32_t max_alt = (config_get_int(&profile.airspace.display.below) * 100) / FC_METER_TO_FEET;

    for (uint32_t i = 0; i < ah.number_of_records; i++)
    {
        airspace_record_t as;
        int32_t rd = red_read(index, &as, sizeof(airspace_record_t));

        if (rd != sizeof(airspace_record_t))
            break;

        //filter out classes or altitude
        if (filter[as.airspace_class] || as.floor > max_alt)
            continue;

        //filter out not in bbox
        if (lon1 > as.bbox.longitude2 || lon2 < as.bbox.longitude1)
            continue;

        if (lat2 > as.bbox.latitude1 || lat1 < as.bbox.latitude2)
            continue;

        memcpy(&fc.airspaces.index[record_index], &as, sizeof(airspace_record_t));

        red_lseek(data, as.points.pos, RED_SEEK_SET);

        if (data_index + (sizeof(gnss_pos_t) * as.number_of_points) >= AIRSPACE_DATA_ALLOC)
        {
            break;
        }

        red_read(data, fc.airspaces.data + data_index, sizeof(gnss_pos_t) * as.number_of_points);
        fc.airspaces.index[record_index].points.ptr = fc.airspaces.data + data_index;
        data_index += sizeof(gnss_pos_t) * as.number_of_points;

        uint8_t name_len = ROUND4((uint32_t)as.name.len);

        if (data_index + name_len >= AIRSPACE_DATA_ALLOC)
        {
            break;
        }

        red_read(data, fc.airspaces.data + data_index, name_len);
        fc.airspaces.index[record_index].name.ptr = fc.airspaces.data + data_index;
        fc.airspaces.index[record_index].name.ptr[name_len] = 0;
        data_index += name_len;

        record_index++;

        if (record_index >= AIRSPACE_INDEX_ALLOC)
        {
            statusbar_msg_add(STATUSBAR_MSG_WARN, "Not all airspaces displayed!");
            break;
        }
    }

    fc.airspaces.number_loaded = record_index;
    fc.airspaces.data_used = data_index;
    red_close(index);
    red_close(data);

    fc.airspaces.valid_lat = c_lat;
    fc.airspaces.valid_lon = c_lon;
    fc.airspaces.valid = true;

    airspace_force_redraw();

    return true;
}

void airspace_unload()
{
    osMutexAcquire(fc.airspaces.lock, WAIT_INF);

    airspace_unload_unlocked();

    osMutexRelease(fc.airspaces.lock);
}

// dot product between AB and AP
#define DOTPRODUCT(A,B,P) ((A.y-B.y)*(P.x-A.x)+(B.x-A.x)*(P.y-A.y))
// the determinant of a,b,c,d
#define DETERMINANT(a,b,c,d) (((a)*(d))-((b)*(c)))

/**
 * Find all airspaces, which are "near" to the pilot. An airspace is
 * considered "near", if the pilot is inside the bounding box of the airspace
 * or the closest point of the airspace to the pilot is less than a specific
 * distance.
 *
 * The result is stored in fc.airspaces.near.
 *
 * @return true, if the near airspaces have changed. False otherwise
 */
bool airspaces_near_find()
{
  int airspaces_near_count = 0;
  bool airspaces_changed = false;

  if (fc.airspaces.valid)
  {
	  for (uint32_t i = 0; i < fc.airspaces.number_loaded; i++)
	  {
		  airspace_record_t * as = &fc.airspaces.index[i];
		  bool airspace_needed = false;

	      // This is used to set a breakpoint to DBG a specific airspace:
		  // if ( strcmp("Stuttgart", as->name.ptr) == 0 && as->airspace_class == ac_class_C)
		  //	  airspace_needed = false;

		  if (fc.gnss.latitude <= as->bbox.latitude1 &&
				  fc.gnss.latitude >= as->bbox.latitude2 &&
				  fc.gnss.longitude >= as->bbox.longitude1 &&
				  fc.gnss.longitude <= as->bbox.longitude2)
		  {
			  // we are inside this airspace
			  airspace_needed = true;
		  }
		  else
		  {
			  // we are outside. How far is it away?
              uint32_t distance_min = geo_distance(fc.gnss.latitude, fc.gnss.longitude, as->bbox.latitude1, fc.gnss.longitude, false, NULL);
              distance_min = min(distance_min, geo_distance(fc.gnss.latitude, fc.gnss.longitude, as->bbox.latitude2, fc.gnss.longitude, false, NULL));
              distance_min = min(distance_min, geo_distance(fc.gnss.latitude, fc.gnss.longitude, fc.gnss.latitude, as->bbox.longitude1, false, NULL));
              distance_min = min(distance_min, geo_distance(fc.gnss.latitude, fc.gnss.longitude, fc.gnss.latitude, as->bbox.longitude2, false, NULL));

			  if (distance_min < 40 * 1000 * 10)                  // 40 km
				  airspace_needed = true;
		  }

		  if (airspace_needed)
		  {
			  airspaces_near_count++;
			  if (airspaces_near_count <= fc.airspaces.near.size)     // enough space in airspaces_near?
			  {
				  if (fc.airspaces.near.asn[airspaces_near_count - 1].as != as)
				  {
					  airspaces_changed = true;
					  fc.airspaces.near.valid = false;
					  bzero(&fc.airspaces.near.asn[airspaces_near_count - 1], sizeof(airspace_near_t));
					  fc.airspaces.near.asn[airspaces_near_count - 1].as = as;
				  }
			  }
		  }
	  }

	  if (airspaces_near_count > fc.airspaces.near.size)
	  {
		  /* "fc.airspaces.near.asn" is too small to hold all near airspaces.
		   * reallocate to a bigger size and copy all existing entries.
		   * The missing entries will be entered again below. */
		  int size_in_bytes_new = airspaces_near_count * sizeof(airspace_near_t);

		  airspace_near_t * airspaces_near_new = ps_malloc(size_in_bytes_new);
		  if (fc.airspaces.near.asn != NULL)
		  {
			  int size_in_bytes_old = fc.airspaces.near.size * sizeof(airspace_near_t);
			  memcpy(airspaces_near_new, fc.airspaces.near.asn, size_in_bytes_old);
			  bzero(airspaces_near_new + size_in_bytes_old, size_in_bytes_new - size_in_bytes_old);
			  ps_free(fc.airspaces.near.asn);
		  }
		  else
		  {
			  bzero(airspaces_near_new, size_in_bytes_new);
		  }
		  fc.airspaces.near.asn = airspaces_near_new;
		  fc.airspaces.near.size = airspaces_near_count;

		  // Array is now big enough: redo
		  airspaces_changed = airspaces_near_find();
	  }
	  else if (airspaces_near_count < fc.airspaces.near.size)
	  {
		  // "airspaces_near" is too big -> bzero remaining space in airspaces_near
		  bzero(&fc.airspaces.near.asn[airspaces_near_count], sizeof(fc.airspaces.near.asn[0]) * (fc.airspaces.near.size-airspaces_near_count));
	  }

	  fc.airspaces.near.num = airspaces_near_count;
  }
  return airspaces_changed;
}

/**
 * Quick sort compare function to sort according int32_t.
 *
 * @param AA pointer to first element
 * @param BB pointer to second element
 *
 * @return -1,0,+1 according qsort specification.
 */
int qsort_distance_cmp(const void *AA, const void *BB)
{
	int32_t *A, *B;

	A = (int32_t *)AA;
	B = (int32_t *)BB;

	if (*A == *B) return 0;
	if (*A < *B) return -1;
	else return +1;
}

/**
 * For a given airsapce_record_t "as" compute all points where the pilot is
 * crossing boundaries of the airspace and return the distances
 * from the pilot to this points. The algorithm is taken from
 * https://math.stackexchange.com/questions/3607924/find-intersection-point-of-line-with-2d-polygon
 *
 * @param pilot   the gnss_pos_t of the pilot
 * @param pilot_B another gnss_pos_t in which direction the pilot is heading to.
 *                Typically derived from fc.gnss.heading by caller.
 * @param as      the airspace, for which we compute the crossings of the pilot.
 *
 * @param distances pointer to a caller allocated array, where we store the
 *                  computed distances. They are returned as "centimeter" values.
 *                  They are negative, if they are behind the pilot and positive
 *                  if they are in front of the pilot.
 *                  All results are sorted in ascending order.
 * @param distances_len the size of the allocated "distances" array given by the caller.
 *
 * @return number of distances computed.
 */
uint8_t airspace_distances(gnss_pos_t pilot, gnss_pos_t pilot_B, airspace_record_t *as, int32_t *distances, uint8_t distances_len)
{
	uint8_t distances_used;
	// char buffer[1000];

	vector_float_t Pi, Pi1, A, B;
	float dx, dy;

	A.x = (float)pilot.longitude / GNSS_MUL;
	A.y = (float)pilot.latitude / GNSS_MUL;

	B.x = (float)pilot_B.longitude / GNSS_MUL;
	B.y = (float)pilot_B.latitude / GNSS_MUL;

	dx = B.x - A.x;
	dy = B.y - A.y;

	// This is used to set a breakpoint to DBG a specific airspace:
	// if ( strcmp("Stuttgart", as->name.ptr) == 0 && as->airspace_class == ac_class_C)
	//	buffer[0] = 0;

	distances_used = 0;
	if (as->number_of_points >= 3)
	{
		Pi.x = (float)as->points.ptr[0].longitude / GNSS_MUL;
		Pi.y = (float)as->points.ptr[0].latitude / GNSS_MUL;

		float dot_i = DOTPRODUCT(A,B,Pi);
		for (int i = as->number_of_points-1; i >= 0; i--)
		{
			// Step 2: For a given index i∈{1,...,n}:
			// Calculate n→*AP→i and n→*AP→i+1
			Pi1.x = (float)as->points.ptr[i].longitude / GNSS_MUL;
			Pi1.y = (float)as->points.ptr[i].latitude / GNSS_MUL;

			float dot_i1 = DOTPRODUCT(A,B,Pi1);

			// DBG("dot_i: %f    dot_i1: %f\n", dot_i, dot_i1);

			// Step 3: If the product (n→*AP→i)(n→*AP→i+1)<0,
			// then the two vertices Pi and Pi+1 are on opposite sides of the
			// line AB and therefore the side PiPi+1 of the polygon intersects
			// the line AB. Proceed to Step 4.
			if (dot_i*dot_i1 < 0)
			{
				// Step 4: In the case of AB intersect PiPi+1, in order to find
				// the intersection point Q=AB∩PiPi+1 of the lines AB and PiPi+1
				vector_float_t Q;

				Q.x =
						DETERMINANT(B.x*A.y-B.y*A.x,       B.x-A.x,
								Pi1.x*Pi.y-Pi1.y*Pi.x, Pi1.x-Pi.x)
								/
								DETERMINANT(A.y-B.y,    B.x-A.x,
										Pi.y-Pi1.y, Pi1.x-Pi.x);

				Q.y =
						DETERMINANT(A.y-B.y,    B.x*A.y-B.y*A.x,
								Pi.y-Pi1.y, Pi1.x*Pi.y-Pi1.y*Pi.x)
								/
								DETERMINANT(A.y-B.y,    B.x-A.x,
										Pi.y-Pi1.y, Pi1.x-Pi.x);
				// Step 5: Calculate the square distance between point A and Q:
				// distance = sqrt((Q.x-A.x)*(Q.x-A.x)+(Q.y-A.y)*(Q.y-A.y));
				int32_t distance = geo_distance(pilot.latitude, pilot.longitude,
						(int32_t)(Q.y * GNSS_MUL), (int32_t)(Q.x * GNSS_MUL),
						false, NULL);

				// Find out, if Q is in the direction of the pilot or behind him.
				float Qdx = Q.x - A.x;
				float Qdy = Q.y - A.y;

				if ( Qdx * dx < 0 || Qdy * dy < 0 )
					distance = -distance;                     // Q is behind pilot

				// sprintf(buffer, "Pi %f,%f  Pi1 %f,%f  Q %f,%f distance %ld", Pi.y, Pi.x, Pi1.y, Pi1.x, Q.y, Q.x, distance );
				// DBG("distance: %d km\n", distance/100000);

				if (distances_used < distances_len)
				{
					distances[distances_used] = distance;
					distances_used++;
				}
				else
				{
					ERR("airspace %s has too many intersections", as->name);
					break;
				}
			}
			Pi = Pi1;
			dot_i = dot_i1;
		}
	}

	if (distances_used > 1)
		qsort(distances, distances_used, sizeof(int32_t), qsort_distance_cmp);

	return distances_used;
}

/**
 * Iterate over all near airspaces and compute the distances to each airspace
 * from the pilot in the direction of flying.
 */
void airspaces_near_compute_distances()
{
	uint8_t distances_used;

	if (fc.airspaces.valid)
	{
		gnss_pos_t pilot, pilot_B;

		pilot.longitude = fc.gnss.longitude;
		pilot.latitude  = fc.gnss.latitude;

		pilot_B.longitude = pilot.longitude + sin(to_radians(fc.gnss.heading)) * GNSS_MUL;
		pilot_B.latitude  = pilot.latitude + cos(to_radians(fc.gnss.heading)) * GNSS_MUL;

		fc.airspaces.near.valid = false;

		// Use each near airspace to compute its distances to the pilot.
		for (uint32_t i = 0; i < fc.airspaces.near.num; i++)
		{
			airspace_record_t * as = fc.airspaces.near.asn[i].as;
			uint8_t distances_len = AIRSPACE_NEAR_DISTANCE_NUM;
			bzero(fc.airspaces.near.asn[i].distances, sizeof(fc.airspaces.near.asn[i].distances));

			// This is used to set a breakpoint to DBG a specific airspace:
			// if ( strcmp("Stuttgart", as->name.ptr) == 0 && as->airspace_class == ac_class_C)
			//	distances_used = 0;

			distances_used = airspace_distances(pilot, pilot_B, as, fc.airspaces.near.asn[i].distances, distances_len);

			/* Find out, if pilot is inside this airspace: */
			int num = 0;
			for (int j = 0; j < distances_used; j++)
			  if (fc.airspaces.near.asn[i].distances[j] < 0) num++;

			// If the number of crossing points behind the pilot is odd, then we are inside
			fc.airspaces.near.asn[i].inside = (num % 2 == 1);
			    
#if DBG_LEVEL == DBG_DBG
		if (distances_used > 0)
		{
			DBG("airspace distance \"%s\": ", as->name.ptr);
			for ( int i = 0; i < distances_used; i++ )
			{
				DBG("%.2f km ", (float)fc.airspaces.near.asn[i].distances[i]/100000);
			}
			DBG("\n");
		}
#endif
		}
	}
}

void airspace_step()
{
    osMutexAcquire(fc.airspaces.lock, WAIT_INF);

    bool airspaces_near_compute_distance = false;

    if (fc.airspaces.valid)
    {
        uint32_t dist = geo_distance(gui.map.lat, gui.map.lon, fc.airspaces.valid_lat, fc.airspaces.valid_lon, true, NULL) / 100;

        if (dist > 64 * 1000)    // 64 km
        {
            INFO("Reloading airspaces...");
            bool ret = airspace_load(config_get_text(&profile.airspace.filename), false);
            if (!ret)
            {
                config_set_text(&profile.airspace.filename, "");
            }
            else
            {
                //airspaces reloaded, near airspaces are not valid
                airspaces_near_compute_distance = true;
                airspaces_near_find();
                DBG("airspace recompute: cache reload");
            }
        }
    }

    /* Check, if we need to recalculate the distances to the airspaces.
     * This is needed if:
     *   - 1 min since last recalculation.
     *   - fc.gnss.heading has changed more than 10 degree
     *   - pilot moved more than 1 km since last recalculation
     */
	int32_t distance = geo_distance(fc.gnss.latitude, fc.gnss.longitude,
			fc.airspaces.near.used_pilot_pos.latitude, fc.airspaces.near.used_pilot_pos.longitude,
			false, NULL);
	if (distance > 100 * 100)   // 100 m
	{
        airspaces_near_find();

		//we need to calculate the positions even when there is no change in near airspaces
        fc.airspaces.near.used_pilot_pos.latitude = fc.gnss.latitude;
        fc.airspaces.near.used_pilot_pos.longitude = fc.gnss.longitude;

        airspaces_near_compute_distance = true;
        DBG("airspace recompute: position change");
	}

    uint32_t now = HAL_GetTick();
    if (now > fc.airspaces.near.last_updated + 1 * 60 * 1000)   // 1 min
    {
    	airspaces_near_compute_distance = true;
    	DBG("airspace recompute: 1 min timeout");
    }
    else
    {
		int16_t current_heading = (int16_t)fc.gnss.heading;
		int16_t diff_heading = (int16_t)abs(fc.airspaces.near.used_heading - fc.gnss.heading) % 360;
		if (diff_heading > 10)
		{
			fc.airspaces.near.used_heading = current_heading;
			airspaces_near_compute_distance = true;
	        DBG("airspace recompute: heading change");
		}
    }

    if (airspaces_near_compute_distance)
    {
    	airspaces_near_compute_distances();
        fc.airspaces.near.last_updated = now;
    }

    fc.airspaces.near.valid = true;

    osMutexRelease(fc.airspaces.lock);
}

void airspace_load_parallel_task(void * param)
{
    osMutexAcquire(fc.airspaces.lock, WAIT_INF);

    if (strcmp(config_get_text(&profile.airspace.filename), "") != 0)
    {
        bool ret = airspace_load(config_get_text(&profile.airspace.filename), false);
        if (!ret)
        {
            config_set_text(&profile.airspace.filename, "");
        }
    }
    else
    {
        airspace_unload_unlocked();
    }

    osMutexRelease(fc.airspaces.lock);

    RedTaskUnregister();
    vTaskDelete(NULL);
}


void airspace_load_parallel()
{
    xTaskCreate((TaskFunction_t)airspace_load_parallel_task, "as_load_parallel_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);
}



